
print("MAX_PROJECTION_STACK_DEPTH is", gl.Get("MAX_PROJECTION_STACK_DEPTH"))
assert(gl.Get("MAX_PROJECTION_STACK_DEPTH") >= 4)

UIParent = {} -- faaaake

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
  local Region_Type_mt = {__index = Region_Type}
  
  -- "gravitate towards center"? how does that work?
  
  -- {"size", value}
  -- {coordinate, target, targ_coordinate, offset}
  
  local function getsize(self, axis)
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
  local function getpoint(self, axis, point)
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
    
    return point * getsize(self, axis)
  end
  local function reanchor(self, axis, token, ...)
    if not self[axis] then
      self[axis] = {[token] = {...}}
    elseif self[axis][token] then
      self[axis][token] = {...}
    else
      local ct = 0
      for _ in pairs(self[axis]) do
        ct = ct + 1
      end
      
      if ct >= 2 then
        print("Too many anchors!")
        print("Trying to anchor", token, ...)
        for anc in pairs(self[axis]) do
          print("Existing anchor", anc)
        end
        assert(ct < 2)
      end
      self[axis][token] = {...}
    end
    
    --getpoint(self, axis, token) -- check for circular dependencies
  end
  

  function Region_Type:Exists()
    return self:GetWidth() and self:GetHeight()
  end
  
  function Region_Type:SetWidth(width)
    reanchor(self, "_anchor_x", "size", width)
  end
  function Region_Type:SetHeight(height)
    reanchor(self, "_anchor_y", "size", height)
  end
  
  function Region_Type:GetWidth()
    local gs = getsize(self, "_anchor_x")
    if type(gs) == "number" then return math.abs(gs) else return gs end
  end
  function Region_Type:GetHeight()
    local gs = getsize(self, "_anchor_y")
    if type(gs) == "number" then return math.abs(gs) else return gs end
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
          break
        end
      end
    end
    self.parent = nil
  end
  function Region_Type:SetLayer(layer)
    self.layer = layer
    self.parent:resort_children()
  end
  function Region_Type:resort_children()
    assert(self.children)
    table.sort(self.children, function (a, b) return (a.layer or 0) < (b.layer or 0) end)
    
    if loud then
    print("layerz")
    for i = 1, #self.children do
      print(self.children[i].layer)
    end
    end
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
  
  function Region_Type:SetAllPoints()
    assert(self.parent)
    reanchor(self, "_anchor_x", 0, self.parent, 0, 0)
    reanchor(self, "_anchor_x", 1, self.parent, 1, 0)
    reanchor(self, "_anchor_y", 0, self.parent, 0, 0)
    reanchor(self, "_anchor_y", 1, self.parent, 1, 0)
    -- grunch
  end
  function Region_Type:ClearAllPoints(not_sizes, not_points)
    assert(not not_points)
    if not_sizes then
      self["_anchor_x"] = {size = self["_anchor_x"].size}
      self["_anchor_y"] = {size = self["_anchor_y"].size}
    else
      self["_anchor_x"], self["_anchor_y"] = nil, nil
    end
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
  
  function Region_Type:SetCoordinateScale(x, y, scale)
    assert((x and y and scale) or not (x or y or scale))
    self.cs_x, self.cs_y, self.cs_scale = x, y, scale
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
  
  function Region_Type:Render()
    if self.hide then return end
    
    if self.bg_r then
      local l, u, r, d = self:GetBounds()
      assert(l and u and r and d)
      gl.Color(self.bg_r, self.bg_g, self.bg_b, self.bg_a)
      gl.Begin("QUADS")
      gl.Vertex(l, u)
      gl.Vertex(r, u)
      gl.Vertex(r, d)
      gl.Vertex(l, d)
      gl.End()
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
    end
    
    if self.Draw then self:Draw() end
    
    if self.children then
      for _, k in ipairs(self.children) do
        k:Render()
      end
    end
    
    if self.cs_x then
      gl.MatrixMode("PROJECTION")
      gl.PopMatrix()
    end
  end
  
  function Region(parent, name, suppress)
    local reg = setmetatable({}, Region_Type_mt)
    if not parent and not suppress then parent = UIParent end
    if parent then reg:SetParent(parent) end
    reg.__name = name
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
  UIParent = UI_CreateParent(1024, 768)
end
UI_Reset()


local function Button_Key(self, button, ascii, event)
  if button ~= "mouse_left" then return true end  -- don't know don't care
  
  if event == "press" then
    self.button_down = true
  elseif event == "release" and self.button_down then
    self.button_down = false
    if self.Click then self:Click() end
  end
end
local function Button_MouseOut(self)
  self.button_down = false
end

local TextOverrides = {}
function TextOverrides:AssimilateBounds()
  self.text:UpdateSize(0, 0)
  self:SetWidth(self.text:GetWidth())
  self:SetHeight(self.text:GetHeight())
end
function TextOverrides:SetSize(size)
  self.text:SetTextSize(size / 1024)
  self:AssimilateBounds()
end
function TextOverrides:SetColor(r, g, b, a)
  Text_SetColor(self.text, r, g, b, a or 1)
end
function TextOverrides:SetText(text)
  self.text:SetText(text)
  self:AssimilateBounds()
end
function TextOverrides:Draw()
  local l, u, r, d = self:GetBounds()
  if not (self.text:GetX() == l and self.text:GetY() == u and self.text:GetClipX1() == l and self.text:GetClipX2() == r and self.text:GetClipY1() == u and self.text:GetClipY2() == d) then
    self.text:SetPosition(l, u, l, u, r, d)
    self.text:UpdateSize(0, 0)
  end
  self.text:Render()
end


local TextMultilineOverrides = {}
function TextMultilineOverrides:SetText(text)
  self.text:SetText("\1J0Cffffffff\1" .. text)
  self.update = true
end
function TextMultilineOverrides:Draw()
  local l, u, r, d = self:GetBounds()
  if self.update or not (self.text:GetX() == l and self.text:GetY() == u and self.text:GetClipX1() == l and self.text:GetClipX2() == r and self.text:GetClipY1() == u and self.text:GetClipY2() == d) then
    self.text:UpdateSize(r - l, d - u)
    self.text:SetPosition(l, u, l, u, r, d)
    self.update = nil
  end
  self.text:Render()
end

local SpriteOverrides = {}
function SpriteOverrides:SetTexture(tex)
  if type(tex) == "string" then tex = Texture(tex) end
  self.tex = tex
  self:SetWidth(tex:GetWidth())
  self:SetHeight(tex:GetHeight())
end
function SpriteOverrides:Draw()
  if self.tex then glutil.RenderBoundedSprite(self.tex, {self:GetBounds()}, self._sprite_r, self._sprite_g, self._sprite_b, self._sprite_a) end
end
function SpriteOverrides:SetColor(r, g, b, a)
  self._sprite_r, self._sprite_g, self._sprite_b, self._sprite_a = r, g, b, a
end

function CreateFrame(typ, parent, name)
  if not name then name = get_stack_entry(2) end
  
  if typ == "Frame" then
    return Region(parent, name)
  elseif typ == "Button" then
    local rg = Region(parent, name)
    rg.Key = Button_Key
    rg.MouseOut = Button_MouseOut
    return rg
  elseif typ == "Text" then
    local rg = Region(parent, name)
    for k, v in pairs(TextOverrides) do
      rg[k] = v
    end
    rg.text = TextFrame_Make("")
    rg:SetText("")
    return rg
  elseif typ == "Text_Multiline" then
    local rg = Region(parent, name)
    for k, v in pairs(TextMultilineOverrides) do
      rg[k] = v
    end
    rg.text = FancyTextFrame_Make("")
    rg:SetText("")
    return rg
  elseif typ == "Sprite" then
    local rg = Region(parent, name)
    for k, v in pairs(SpriteOverrides) do
      rg[k] = v
    end
    return rg
  else
    assert(false)
  end
  assert(false)
end

local function TraverseUpWorker(start, x, y, keyed)
  if start.hide then return end
  
  if start.children then
    for tid = #start.children, 1, -1 do
      local k = start.children[tid]
      local rv = TraverseUpWorker(k, x, y, keyed or start.Key)
      if rv then return rv end
    end
  end
  
  if start:Exists() and (keyed or start.Key) and x >= start:GetLeft() and x < start:GetRight() and y >= start:GetTop() and y < start:GetBottom() then
    return start
  end
  
  return nil
end

function TraverseUp(val)
  local x, y = GetMouse()
  
  local rv = TraverseUpWorker(parents[val].parent, x / 1024 * parents[val].width, y / 768 * parents[val].height, false)
  
  return rv
end

function AccumulateInternals(start, acu, x, y)
  if not start then start = UIParent end
  if not acu then acu = {} end
  
  if not x then x, y = GetMouse() end
  
  if start.children then
    for _, k in ipairs(start.children) do
      AccumulateInternals(k, acu, x, y)
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
    local frame_target = TraverseUp(k)
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
    if IsKeyDownFrame(k) then
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
  local this_inside = AccumulateInternals()
  
  for k in pairs(last_inside) do
    if not this_inside[k] and k.MouseOut then k:MouseOut() end
  end
  for k in pairs(this_inside) do
    if not last_inside[k] and k.MouseIn then k:MouseIn() end
  end
  
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
        if IsKeyDownFrame(k) then
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
