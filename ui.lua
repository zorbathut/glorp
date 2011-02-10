
--print("MAX_PROJECTION_STACK_DEPTH is", gl.Get("MAX_PROJECTION_STACK_DEPTH"))
--assert(gl.Get("MAX_PROJECTION_STACK_DEPTH") >= 4)

UIParent = {} -- faaaake

--[[local cache_statistics = {}
function UI_cache_statistics()
  local boil = {}
  for k, v in pairs(cache_statistics) do
    table.insert(boil, {k = k, v = v})
  end
  table.sort(boil, function (a, b) return a.v > b.v end)
  for _, v in ipairs(boil) do
    print(v.v, v.k)
  end
  cache_statistics = {}
end]]


local function ScreenToLocal(frame, lx, ly)
  if frame.cs_x then
    -- now we have to do some transformations
    -- first, figure out where we are within the coordinate space of the window
    lx = (lx - (frame:GetLeft() + frame:GetRight()) / 2) / frame:GetWidth()
    ly = (ly - (frame:GetTop() + frame:GetBottom()) / 2) / frame:GetWidth()
    
    -- now we rotate (we don't rotate yet)
    
    -- then we rescale
    lx = lx * frame.cs_scale + frame.cs_x
    ly = ly * frame.cs_scale + frame.cs_y
  end
  
  return lx, ly
end

-- Width and height: unless set, they're the default size (currently 5).
-- If the region is width-locked or height-locked thanks to anchor points, then trying to set the width or height is an error.
-- Setting up any kind of a circular dependency is also an error, although note that surprisingly few things are circular - "circular dependencies" that touch different anchor axes may not actually be circular. For example,
-- Container is the parent of Button
-- Button UL is anchored to Container UL
-- Container BR is anchored to Button BR
-- Bam, we have a container that adapts to fit its contents
do
  -- Locals
    -- Axes, split into L R T B
      -- Check to see if it's anchored - anchor_l, anchor_l_point, and anchor_l_offset
      -- If so, return the appropriately modified thing
      -- If not, check to see if the alternatives (r, cx) are anchored, then return the modified version based on the width
      -- If not, assume we're anchored to 0,0
    -- Width, split into H W
      -- Check to see if the two points are anchored, then return different
      -- If not, return the internal size_w size_h
  
  local Region_Type = {}
  local Region_Type_mt = {__index = Region_Type, __tostring = function (self) return "<frame " .. (self.__name or "(unnammed)") .. " " .. toaddress(self) .. ">" end}
  
  -- bit of musing on caching
  -- we'll have to maintain a "layout children" list, and deal with that properly on attach/detach (make it weak pointers so we can ignore it with attach/detach)
  -- when anything is looked up, size or point, we check the cache first. when it's inserted, we insert it into the cache.
  
  -- "gravitate towards center"? how does that work? answer: it doesn't, let's not do that
  
  -- {"size", value}
  -- {coordinate, target, targ_coordinate, offset}
  
  local function getsize_core(self, axis)
    --print("gs", self, axis)
    if self[axis] then
      if self[axis].size then
        return self[axis].size[1]
      end
      
      local ptx, ptxl = next(self[axis])
      local pty, ptyl = next(self[axis], ptx)
      
      if ptx and pty then
        local ptxp = ptxl[1]:GetPointOnAxis(axis, ptxl[2]) + ptxl[3]
        local ptyp = ptyl[1]:GetPointOnAxis(axis, ptyl[2]) + ptyl[3]
        
        return (ptyp - ptxp) / (pty - ptx)
      end
    end
    
    return self.bg_r and 20 -- everything is 20, unless it actually fails to exist
  end
  local function getsize(self, axis)
    if not self.__cache[axis] then
      self.__cache[axis] = {}
    end
    if not self.__cache[axis].size then
      self.__cache[axis].size = getsize_core(self, axis) or "nil"
      --[[if cache_statistics then
        cache_statistics[self.__name] = (cache_statistics[self.__name] or 0) + 1
      end]]
    end
    return tonumber(self.__cache[axis].size)
  end
  local function getpoint_core(self, axis, point)
    --print("GPC", self)
    assert(axis)
    assert(point)
    
    --print("gp", self, axis, point)
    if self[axis] then
      do
        local tp = self[axis][point]
        if tp then return tp[1]:GetPointOnAxis(axis, tp[2]) + tp[3] end
      end
      
      -- Various options:
      -- * Zero points and size
      -- * At least one point and size
      -- There's always an implicit size, at the very worst. Two points degenerates to one point, since we need the size anyway. We'll need caching at some point or this is going to be n^3 or so.
      
      local ptx, ptxl = next(self[axis])
      local pty, ptyl = next(self[axis], ptx)
      
      if ptx == "size" then
        ptx, ptxl, pty, ptyl = pty, ptyl, nil, nil
      end
      
      if ptx then
        local ptxp = ptxl[1]:GetPointOnAxis(axis, ptxl[2]) + ptxl[3]
        local wid = getsize(self, axis)
        assert(wid)
        return ptxp + (point - ptx) * wid
      end -- zero points falls back to no constraints
    end
    
    local gs = getsize(self, axis)
    if not gs then
      print("Failure to get size for " .. tostring(self.__name) .. " axis " .. tostring(axis))
      assert(gs)
    end
    
    return point * gs
  end
  local function getpoint(self, axis, point)
    if not self.__cache[axis] then
      self.__cache[axis] = {}
    end
    if not self.__cache[axis][point] then
      self.__cache[axis][point] = getpoint_core(self, axis, point) or "nil"
      --[[if cache_statistics then
        cache_statistics[self.__name] = (cache_statistics[self.__name] or 0) + 1
      end]]
    end
    return tonumber(self.__cache[axis][point])
  end
  
  local axisanchormeta = {__mode = 'k'}
  local function new_axis_anchor()
    return setmetatable({}, axisanchormeta)
  end
  
  local notifyMap = {
    _anchor_x = "OnShiftX",
    _anchor_y = "OnShiftY",
  }
  local function decache(self, axis)
    if self.__cache[axis] then
      self.__cache[axis] = nil
      
      for k in pairs(self.__anchor_children[axis]) do
        decache(k, axis)
      end
    end
    
    local notifyfunc = notifyMap[axis]
    if self[notifyfunc] then self[notifyfunc](self) end
  end
  
  local function reanchor(self, axis, token, grip, p1, p2)
    if not self[axis] and not grip then
      -- we "clear", but we clear something that doesn't actually exist
    elseif not self[axis] then
      self[axis] = {[token] = {grip, p1, p2}}
    elseif self[axis][token] then
      if self[axis][token][1] == grip and self[axis][token][2] == p1 and self[axis][token][3] == p2 then
        -- it is being set to the same thing! let's just do nothing
        return  -- skip the decaching
      end
      
      if type(self[axis][token][1]) == "table" then
        self[axis][token][1].__anchor_children[axis][self] = nil
      end
      if not grip then
        self[axis][token] = nil
      else
        self[axis][token] = {grip, p1, p2}
      end
    elseif not grip then
      -- we "clear", but we clear something that doesn't actually exist
      return  -- skip the decaching
    else
      local ct = 0
      for _ in pairs(self[axis]) do
        ct = ct + 1
      end
      
      if ct >= 2 then
        print("Too many anchors!")
        print("Trying to anchor", token, grip, p1, p2)
        for anc in pairs(self[axis]) do
          print("Existing anchor", anc)
        end
        assert(ct < 2)
      end
      self[axis][token] = {grip, p1, p2}
    end
    
    if type(grip) == "table" then
      grip.__anchor_children[axis][self] = true
    end
    
    decache(self, axis)
    --getpoint(self, axis, token) -- check for circular dependencies
  end
  
  -- hmm is this a good idea?
  function Region_Type:GetRelativeMousePosition()
    return ScreenToLocal(self, self.parent:GetRelativeMousePosition())
  end
  --[[function Region_Type:IsMouseInside()  -- may not work
    local x, y = self:GetRelativeMousePosition()
    return x >= self:GetLeft() and x <= self:GetRight() and x >= self:GetTop() and x <= self:GetBottom()
  end]]

  function Region_Type:Exists()
    return self:GetWidth() and self:GetHeight()
  end
  
  function Region_Type:SetWidth(width)
    reanchor(self, "_anchor_x", "size", width)
  end
  function Region_Type:SetHeight(height)
    reanchor(self, "_anchor_y", "size", height)
  end
  function Region_Type:SetSize(width, height)
    if not height then height = width end
    self:SetWidth(width)
    self:SetHeight(height)
  end
  
  function Region_Type:GetWidth()
    local gs = getsize(self, "_anchor_x")
    if gs then return math.abs(gs) else return gs end
  end
  function Region_Type:GetHeight()
    local gs = getsize(self, "_anchor_y")
    if gs then return math.abs(gs) else return gs end
  end
  function Region_Type:GetSize()
    return self:GetWidth(), self:GetHeight()
  end
  
  function Region_Type:GetLeft()
    return getpoint(self, "_anchor_x", 0)
  end
  function Region_Type:GetRight()
    return getpoint(self, "_anchor_x", 1)
  end
  function Region_Type:GetTop()
    return getpoint(self, "_anchor_y", 0)
  end
  function Region_Type:GetBottom()
    return getpoint(self, "_anchor_y", 1)
  end
  
  function Region_Type:GetXCenter()
    return getpoint(self, "_anchor_x", 0.5)
  end
  function Region_Type:GetYCenter()
    return getpoint(self, "_anchor_y", 0.5)
  end
  function Region_Type:GetCenter()
    return self:GetXCenter(), self:GetYCenter()
  end
  function Region_Type:GetTopLeft()
    return self:GetLeft(), self:GetTop()
  end
  
  function Region_Type:GetBounds()
    return self:GetLeft(), self:GetTop(), self:GetRight(), self:GetBottom()
  end
  
  function Region_Type:GetPointOnAxis(axis, pt)
    if verbositude then print(self, axis, pt) end
    if axis == "x" or axis == "_anchor_x" then
      return getpoint(self, "_anchor_x", pt)
    elseif axis == "y" or axis == "_anchor_y" then
      return getpoint(self, "_anchor_y", pt)
    else
      print("Weird axis:", axis)
      assert(false)
    end
  end
  
  function Region_Type:SetParent(parent)
    self:Detach()
    
    self.parent = parent
    
    if parent then
      if not parent.children then parent.children = {} end
      table.insert(parent.children, self)
      
      parent:resort_children()
    end
  end
  function Region_Type:Detach()
    if self.parent then
      for k, v in ipairs(self.parent.children) do
        if v == self then
          table.remove(self.parent.children, k)
          self.parent._last_updated = nil -- we might need to reprocess
          break
        end
      end
    end
    self.parent = nil
    
    -- Note: Technically we're still anchored. This is A-OK, we might reattach to something else later on. If we get GC'ed, well, that's why the anchor children tables are weak-keyed
  end
  function Region_Type:SetLayer(layer)
    self.layer = layer
    self.parent:resort_children()
  end
  function Region_Type:resort_children()
    assert(self.children)
    table.sort(self.children, function (a, b)
      local al, bl = a.layer or 0, b.layer or 0
      if al ~= bl then return al < bl end
      return a.__globid < b.__globid
    end)
    self._last_updated = nil -- we might need to reprocess
    --[[
    if loud then
      print("layerz")
      for i = 1, #self.children do
        print(self.children[i].layer)
      end
    end]]
  end
  
  function Region_Type:Hide()
    self.hide = true
  end
  function Region_Type:Show(show)
    if show == nil then
      self.hide = nil
    else
      self.hide = not show
    end
  end
  function Region_Type:IsShown()
    return not self.hide
  end
  
  function Region_Type:SetAllPoints(target)
    target = target or self.parent
    assert(target)
    reanchor(self, "_anchor_x", 0, target, 0, 0)
    reanchor(self, "_anchor_x", 1, target, 1, 0)
    reanchor(self, "_anchor_y", 0, target, 0, 0)
    reanchor(self, "_anchor_y", 1, target, 1, 0)
    -- grunch
  end
  
  local function clear(self, axis, nsize, npoint)
    if not self[axis] then return end
    
    local cle = {}
    for k in pairs(self[axis]) do
      if k == "size" then
        if not nsize then
          table.insert(cle, k)
        end
      else
        if not npoint then
          table.insert(cle, k)
        end
      end
    end
    
    for _, v in ipairs(cle) do
      reanchor(self, axis, v, nil)
    end
  end
  
  function Region_Type:ClearAllPoints(not_sizes, not_points)
    clear(self, "_anchor_x", not_sizes, not_points)
    clear(self, "_anchor_y", not_sizes, not_points)
  end
  
  local function set_axis(self, x, y, target, tx, ty, ofsx, ofsy)
    assert(target)
    assert(target ~= self)
    --print(self, x, y, target, tx, ty, ofsx, ofsy)
    if x and tx then reanchor(self, "_anchor_x", x, target, tx, ofsx or 0) end
    if y and ty then reanchor(self, "_anchor_y", y, target, ty, ofsy or 0) end
  end
  
  local strconv = {
    TOPLEFT = {0, 0},
    TOPRIGHT = {1, 0},
    BOTTOMLEFT = {0, 1},
    BOTTOMRIGHT = {1, 1},
    CENTER = {0.5, 0.5},
    LEFT = {0, nil},
    RIGHT = {1, nil},
    TOP = {nil, 0},
    BOTTOM = {nil, 1},
  }
  function Region_Type:SetPoint(a, b, c, d, e, f, g)
    if type(a) == "string" then
      if strconv[a] then
        return self:SetPoint(strconv[a][1], strconv[a][2], b, c, d, e, f, g)
      else
        assert(false)
      end
    elseif type(d) == "string" then
      if strconv[d] then
        return self:SetPoint(a, b, c, strconv[d][1], strconv[d][2], e, f, g)
      else
        assert(false)
      end
    else
      return set_axis(self, a, b, c, d, e, f, g)
    end
  end
  
    
  -- let's think about the set-coordinate-scale function for a bit
  -- I want to be able to do things like
  -- SetCoordinateScale(x, y, scale)
  -- I want this to mean "move the center to x, y and set the scale to q". I think.
  -- Questions: What does "center" mean?
  -- I think "center" is relative to the frame center from the last coordinate system. At least I can't think of anything better for it to mean.
  -- "scale", similar deal.
  -- okay it turns out all of that is wrong
  -- "center" is the new origin of the new coordinate system
  -- "scale" is the width of the new coordinate system (it ends up being the size of the window)
  -- "rotate" is in degrees I guess
  
  function Region_Type:SetCoordinateScale(x, y, scale, rotate)
    assert((x and y and scale) or not (x or y or scale))
    self.cs_x, self.cs_y, self.cs_scale, self.cs_rotate = x, y, scale, rotate
  end
  
  
  function Region_Type:SetBackgroundColor(r, g, b, a)
    if not r and not g and not b and not a then a = 0 end
    if a == 0 then
      self.bg_r, self.bg_g, self.bg_b, self.bg_a = nil, nil, nil, nil
      return
    end
    
    assert(r and g and b)
    if not a then a = 1 end
    
    self.bg_r, self.bg_g, self.bg_b, self.bg_a = r, g, b, a
  end
  function Region_Type:GetBackgroundColor()
    return self.bg_r, self.bg_g, self.bg_b, self.bg_a
  end
  
  function Region_Type:Render()
    if self.hide then return end
    
    if self.bg_r then
      local l, u, r, d = self:GetBounds()
      assert(l and u and r and d)
      gl.Color(self.bg_r, self.bg_g, self.bg_b, self.bg_a)
      glutil.RenderArray("TRIANGLE_FAN", 2, {l, u, r, u, r, d, l, d})
    end
    
    if self.cs_x then
      gl.MatrixMode("PROJECTION")

      gl.PushMatrix()
      
      local sx, sy, ex, ey = self:GetBounds()
      local cx, cy = (sx + ex) / 2, (sy + ey) / 2
      local scalefact = (ex - sx) / self.cs_scale
      gl.Translate(cx, cy, 0)
      gl.Scale(scalefact, scalefact, 1)
      gl.Translate(-self.cs_x, -self.cs_y, 0)
      if self.cs_rotate then gl.Rotate(self.cs_rotate / 3.14159 * 180, 0, 0, 1) end
    end
    
    if self.DrawPre then self:DrawPre() end
    
    if not self.DrawSkip then
      if self.Draw then self:Draw() end
      
      if self.children then
        for _, k in ipairs(self.children) do
          k:Render()
        end
      end
    end
    
    if self.DrawPost then self:DrawPost() end
    
    if self.cs_x then
      gl.MatrixMode("PROJECTION")
      gl.PopMatrix()
    end
  end
  
  function Region_Type:Update(quanta)
    if self._last_updated ~= quanta then
      if self.Tick then self:Tick() end
    end
    
    while self._last_updated ~= quanta do
      self._last_updated = quanta
      
      if self.children then
        for _, k in ipairs(self.children) do
          k:Update(quanta)
        end
      end
    end
  end
  
  local globid = 0
  local weak_key_meta = {__mode = 'k'}
  function Region(parent, name, suppress)
    local reg = setmetatable({}, Region_Type_mt)
    if not parent and not suppress then parent = UIParent end
    reg.__name = name
    reg.__cache = {_anchor_x = {}, _anchor_y = {}}
    reg.__anchor_children = {_anchor_x = setmetatable({}, weak_key_meta), _anchor_y = setmetatable({}, weak_key_meta)}
    reg.__globid = globid
    globid = globid + 1
    if parent then reg:SetParent(parent) end
    return reg
  end
end

local parents = {}

function UI_CreateParent(width, height)
  local parent = Region(nil, get_stack_entry(2), true)
  parent:Detach()
  function parent:GetWidth()
    return width
  end
  function parent:GetHeight()
    return height
  end
  function parent:GetLeft()
    return 0
  end
  function parent:GetRight()
    return self:GetWidth()
  end
  function parent:GetTop()
    return 0
  end
  function parent:GetBottom()
    return self:GetHeight()
  end
  function parent:GetPointOnAxis(axis, pt)
    if axis == "x" or axis == "_anchor_x" then return pt * self:GetWidth() end
    if axis == "y" or axis == "_anchor_y" then return pt * self:GetHeight() end
  end
  function parent:GetRelativeMousePosition()
    return GetMouse()
  end
  parent.parent = nil
  table.insert(parents, {parent = parent, width = width, height = height})
  return parent
end
function UI_ReregisterParent(parent, width, height) -- ughhhh
  table.insert(parents, {parent = parent, width = width, height = height})
end

function UI_Reset()
  UIParent = nil
  parents = {}
  UIParent = UI_CreateParent(GetScreenX(), GetScreenY())
end
UI_Reset()

local FrameTypes = {}
FrameTypes.Frame = {}

runfile("ui_widgets.lua", _G, FrameTypes)

function CreateFrame(typ, parent, name)
  if not name then
    local id = 2
    name = get_stack_entry(2)
    while name:find("(tail call)") do
      id = id + 1
      name = get_stack_entry(id)
    end
  end
  
  assert(FrameTypes[typ], typ)
  
  local rg = Region(parent, name)
  for k, v in pairs(FrameTypes[typ]) do
    if k ~= "_Init" then
      rg[k] = v
    end
  end
  
  if FrameTypes[typ]._Init then
    FrameTypes[typ]._Init(rg, name)
  end
  
  return rg
end

local function TraverseUpWorker(start, x, y, keyed)
  if not start:IsShown() then return end
  
  if start.key_bounds_cull then
    assert(start:Exists())
    if not (x >= start:GetLeft() and x < start:GetRight() and y >= start:GetTop() and y < start:GetBottom()) then
      return nil
    end
  end
  
  if start.children then
    local lx, ly = ScreenToLocal(start, x, y)
    
    for tid = #start.children, 1, -1 do
      local k = start.children[tid]
      local rv = TraverseUpWorker(k, lx, ly, keyed or start.Key)
      if rv then return rv end
    end
  end
  
  if start:Exists() and (keyed or start.Key) and x >= start:GetLeft() and x < start:GetRight() and y >= start:GetTop() and y < start:GetBottom() then
    return start
  end
  
  return nil
end

function TraverseUp(val, button)
  local finger_id = button and button:match("finger_(%d+)")
  
  local x, y
  
  if not finger_id then
    -- we are doing a normal mouse or keyboard thing
    x, y = GetMouse()
  else
    -- holy shit we are running on an iphone
    -- can you believe that
    -- a motherfucking iphone
    -- jesus goddamn christ
    finger_id = tonumber(finger_id)
    assert(finger_id)
    
    x, y = touch_getX(finger_id), touch_getY(finger_id)
    
    print("finger", finger_id, x, y)
  end
  
  local rv = TraverseUpWorker(parents[val].parent, x / GetScreenX() * parents[val].width, y / GetScreenY() * parents[val].height, false)
  
  return rv
end

function AccumulateInternals(start, acu, x, y)
  if not start then start = UIParent end
  if not acu then acu = {} end
  if not start:IsShown() then return acu end
  
  if not x then x, y = GetMouse() end
  
  if start.children then
    local lx, ly = ScreenToLocal(start, x, y)
      
    for _, k in ipairs(start.children) do
      AccumulateInternals(k, acu, lx, ly)
    end
  end
  
  if start:Exists() and x >= start:GetLeft() and x < start:GetRight() and y >= start:GetTop() and y < start:GetBottom() then
    acu[start] = true
  end
  
  return acu
end

local focus

function GetFocus() return focus end
function SetFocus(nfocus) focus = nfocus end

local kiiz = {}

function UI_Key(button, ascii, event)
  if button and ascii then
    kiiz[button] = ascii or true
  end
  local passdown = true
  
  if focus and not focus:IsShown() then focus = nil end -- shoo
  
  -- If we have focus, and it was a useful key, then we send it to focus to see if the focus wants to eat its face
  -- eat. its face. eat.
  if ascii and focus then
    passdown = focus:Key(button, ascii, event)
  end
  if not passdown then return end
  
  for k in ipairs(parents) do
    local frame_target = TraverseUp(k, button)
    -- Otherwise, we first figure out what the topmost frame it's on is, with topmost being simply defined as "last in the render order but with the key over it".
    -- Then we traverse up, looking for something that wants to eat it.
    while frame_target do
      if frame_target.Key then
        passdown = frame_target:Key(button, ascii, event)
        if not passdown then return end
      end
      frame_target = frame_target.parent
    end
  end
  
  -- Nobody cares. Send it to the global handler, if one exists.
  if key then key(button, ascii, event) end
end

local coroutine_background = {}
function coroutine.background(func) -- might skip a frame or two if other coros are running
  table.insert(coroutine_background, coroutine.wrap(func))
end



local important_keys = {}
function WasKeyPressed(key)
  if not important_keys[key] then important_keys[key] = "down" return false end
  --print(key, important_keys[key])
  return important_keys[key] == "pressed"
end

function KeyPressed_Update()
  for k in pairs(important_keys) do
    if IsKeyDown(k) then
      if important_keys[k] == "up" then important_keys[k] = "pressed" end
    else
      important_keys[k] = "up"
    end
  end
end

function KeyPressed_Cycle()
  for k in pairs(important_keys) do
    if important_keys[k] == "pressed" then important_keys[k] = "down" end
  end
end

local frems = 0
local tickaccum = 0
local frame = 1000 / 60

local last_inside = {}
function UI_Loop(tix, ...)
  alutil.Tick()
  
  local this_inside
  perfbar(0, 0.5, 0.5, function ()
    this_inside = AccumulateInternals()
  end)
  
  perfbar(0.5, 0.0, 0.5, function ()
    for k in pairs(last_inside) do
      if not this_inside[k] and k.MouseOut then k:MouseOut() end
    end
    for k in pairs(this_inside) do
      if not last_inside[k] and k.MouseIn then k:MouseIn() end
    end
  end)
  
  last_inside = this_inside
  
  if loop then loop(tix, ...) end
  
  for k, v in pairs(coroutine_background) do
    local rv = v()
    if rv then table.remove(coroutine_background, k) end
  end
  
  
  if tick_loop then
    tickaccum = tickaccum + tix
    KeyPressed_Update()
    while tickaccum > frame do
      tickaccum = tickaccum - frame
      
      for k, v in pairs(kiiz) do
        if IsKeyDown(k) then
          if v == true then
            UI_Key(k, nil, "frame")
          else
            UI_Key(k, v, "frame")
          end
        end
      end
      
      tick_loop()
      
      KeyPressed_Cycle()
    end
  end
end
