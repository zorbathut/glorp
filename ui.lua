
--print("MAX_PROJECTION_STACK_DEPTH is", gl.Get("MAX_PROJECTION_STACK_DEPTH"))
--assert(gl.Get("MAX_PROJECTION_STACK_DEPTH") >= 4)

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

-- Let's think about the semantics here
-- Internal rescaling and origin rejiggering should be done for the children only, it creates problems otherwise
-- Semantics/documentation:

-- SetPoint((coord/coordpair), ((parent, (coord/coordpair))/"origin"), xofs, yofs)
-- offsets are all based on the local coordinate scale
-- positions are absolute
-- when we calculate positional changes, we find the absolute loc, then the relative loc

-- in order to rescale, we can have a few manners, similar to how size is calculated
-- either you can state the new scale/origin explicitly, or you can state size and two points and origin
-- origin: SetPoint semantics? SetPoint("ORIGIN", selfframe, "CENTER", x, y)? Maybe SetPoint("ORIGINX", etc) eventually
-- rotation is separate from that
-- We also need a bool for allowing the rescale stuff to happen

-- So, rescale-related functionality:
-- PermitAutoScale boolean
-- SetScale, similar to SetSize
-- Not gonna bother with x/y yet
-- Let's rename Text::SetSize to SetTextSize

-- Dependencies:
-- We have some categories of thing. Point hooks are _anchor_x and _anchor_y? origins are separate, because origin can depend on
-- oh wait that doesn't seem to work well, since origin can point at "self", and points can depend on "origin"
-- hmmmmm
-- let's ignore dependencies for now. start with no caching add frame caching go from there
do
  local weak_key = {__mode = "k"}
  local weak_value = {__mode = "v"}
  local weak_keyvalue = {__mode = "kv"}
  
  local Node_Type = {}
  local Node_Type_mt = {__index = Node_Type, __tostring = function (self) return self.__name end}
  
  local invalidations = 0
  
  function Node_Type:Set(dependencies, command)
    self:Invalidate()
    
    assert(command)
    
    self:ReplaceDependencies(dependencies)
    self.command = command
  end
  function Node_Type:ReplaceDependencies(dependencies)
    if self.dependencies then
      for _, v in pairs(self.dependencies) do
        v:RemoveChild(self)
      end
    end
    
    self.dependencies = dependencies
    
    if self.dependencies then
      for _, v in pairs(self.dependencies) do
        v:AddChild(self)
      end
    end
  end
  function Node_Type:Get()
    if self.cached then return self.cache end
    assert(not self.executing)  -- loop detection, error more usefully
    self.executing = true
    self.cached = true
    self.cache = self.command(self.dependencies)
    self.executing = false
    return self.cache
  end
  function Node_Type:Invalidate()
    if self.cached then
      invalidations = invalidations + 1
      self.cache = nil
      self.cached = false
      for k in pairs(self.children) do
        k:Invalidate()
      end
    end
  end
  function Node_Type:AddChild(child)
    self.children[child] = (self.children[child] or 0) + 1
  end
  function Node_Type:RemoveChild(child)
    self.children[child] = (self.children[child] or 0) - 1
    assert(self.children[child] >= 0)
    if self.children[child] == 0 then
      self.children[child] = nil
    end
  end
  function CreateNode(name)
    local nod = setmetatable({}, Node_Type_mt)
    nod.children = setmetatable({}, weak_key)
    nod.__name = name
    return nod
  end
  
  function instrumentation_GetInvalidations()
    return invalidations
  end
  function instrumentation_ResetInvalidations()
    invalidations = 0
  end
  
  local Axis_Type = {}
  local Axis_Type_mt = {__index = Axis_Type, __tostring = function (self) return self.__name end}
  
  function Axis_Type:Get(point)
    if self.explicit[point] then return self.explicit[point] end
    if self.implicit[point] then return self.implicit[point] end
    
    self.implicit[point] = CreateNode(self.__name .. " " .. tostring(point))
    
    self:Remake(point)
    
    return self.implicit[point]
  end
  function Axis_Type:Set(point, dependencies, command)
    if not self.explicit[point] then
      if point == "size" then
        assert(self.exppoint < 2)
        self.expsize = true
      else
        assert(self.exppoint < 2)
        assert(not self.expsize or self.exppoint < 1)
        self.exppoint = self.exppoint + 1
      end
      
      if self.implicit[point] then
        self.explicit[point] = self.implicit[point]
        self.implicit[point] = nil
      else
        self.explicit[point] = CreateNode(self.__name .. " " .. tostring(point))
      end
      
      self:RemakeAll()
    end
    
    self.explicit[point]:Set(dependencies, command)
  end
  function Axis_Type:Clear(point)
    if self.explicit[point] then
      self.implicit[point] = self.explicit[point]
      self.explicit[point] = nil
      
      if point == "size" then
        self.expsize = false
      else
        self.exppoint = self.exppoint - 1
      end
      
      self:RemakeAll()
    end
  end
  function Axis_Type:ClearPoints(point)
    if point == "all" then
      self:ClearAll()
    elseif point == "alignment" then
      while true do
        local nk = next(self.explicit)
        if nk == "size" then nk = next(self.explicit, nk) end
        if not nk then break end
        self:Clear(nk)
      end
    else
      self:Clear(point)
    end
  end
  function Axis_Type:ClearAll()
    while next(self.explicit) do
      self:Clear(next(self.explicit))
    end
    
    assert(not self.expsize)
    assert(self.exppoint == 0)
  end
  function Axis_Type:RemakeAll()
    for k in pairs(self.implicit) do
      self:Remake(k)
    end
  end
  function Axis_Type:Remake(point)
    if point == "size" then
      if self.exppoint < 2 then
        self.implicit[point]:Set({}, function () return self.defsize end)  -- default values
      elseif self.exppoint == 2 then
        -- default values don't work, we're gonna have to calculate
        local deps = {}
        local ka, va = next(self.explicit)
        local kb, vb = next(self.explicit, ka)
        assert(va)
        assert(vb)
        table.insert(deps, va)
        table.insert(deps, vb)
        self.implicit[point]:Set(deps, function () return (vb:Get() - va:Get()) / (kb - ka) end)
      else
        assert(false)
      end
    else
      if self.exppoint == 0 then
        -- we have essentially nothing to go on
        self.implicit[point]:Set({self:Get("size")}, function (params) return point * params[1]:Get() end)
      else
        local ka, va = next(self.explicit)
        if ka == "size" then ka, va = next(self.explicit, ka) end
        assert(ka and va)
        self.implicit[point]:Set({self:Get("size"), va}, function (params) assert(va:Get()) return va:Get() + (point - ka) * params[1]:Get() end)
      end
    end
  end
  local function CreateAxis(name, default_size)
    local axi = setmetatable({}, Axis_Type_mt)
    axi.__name = name
    axi.explicit = {}
    axi.implicit = setmetatable({}, weak_value)
    axi.expsize = false
    axi.exppoint = 0
    axi.defsize = default_size
    return axi
  end
  
  local axis_conversions = {x = setmetatable({}, weak_keyvalue), y = setmetatable({}, weak_keyvalue)}
  local function GetAxisConversion(axis, src, dst)
    local subs = axis_conversions[axis]
    local tab = subs[src] -- cache to keep it around in case the GC happens to hit at just the wrong moment
    if not tab then
      tab = setmetatable({}, weak_keyvalue)
      subs[src] = tab
    end
    
    if not tab[dst] then
      local oaxis = "origin" .. axis
        
      local nod = CreateNode(string.format("axis conversion (%s) (%s)", tostring(src), tostring(dst)))
      tab[dst] = nod
      nod.backlink = tab  -- to keep this table from going away
      
      local function generatePath(item)
        local path = {}
        while item do
          table.insert(path, item)
          item = item.__parent_node:Get()
        end
        return path
      end
      
      nod:Set({}, function ()
        local srcpath = generatePath(src.__parent_node:Get())
        local dstpath = generatePath(dst.__parent_node:Get())
        
        while #srcpath > 0 and #dstpath > 0 and srcpath[#srcpath] == dstpath[#dstpath] do
          table.remove(srcpath)
          table.remove(dstpath)
        end
        
        local deps = {src.__parent_node, dst.__parent_node}
        
        local ofs = 0
        local scale = 1
        
        for i = 1, #srcpath do
          local item = srcpath[i]
          local lofs = item.__axes[oaxis]:Get(0)
          local lscale = item.__axes[oaxis]:Get("size")
          table.insert(deps, lofs)
          table.insert(deps, lscale)
          table.insert(deps, item.__parent_node)
          
          ofs = ofs * lscale:Get() + lofs:Get()
          scale = scale * lscale:Get()
        end
        
        for i = #dstpath, 1, -1 do
          local item = dstpath[i]
          local lofs = item.__axes[oaxis]:Get(0)
          local lscale = item.__axes[oaxis]:Get("size")
          table.insert(deps, lofs)
          table.insert(deps, lscale)
          table.insert(deps, item.__parent_node)
          
          ofs = (ofs - lofs:Get()) / lscale:Get()
          scale = scale / lscale:Get()
        end
        
        nod:ReplaceDependencies(deps)
        
        return {offset = ofs, scale = scale}
      end)
    end
    
    return subs[src][dst]
  end
  
  local Region_Type = {}
  local Region_Type_mt = {__index = Region_Type, __tostring = function (self) return "<frame " .. (self.__name or "(unnamed)") .. " " .. toaddress(self) .. ">" end}
  
  function Region_Type:GetHandle(axis, coord)
    return self.__axes[axis]:Get(coord)
  end
  function Region_Type:SetHandle(axis, coord, deps, func)
    return self.__axes[axis]:Set(coord, deps, func)
  end
  
  function Region_Type:ClearPoints(axes, types)
    if axes == "alignment" then
      self:ClearPoints("x", types)
      self:ClearPoints("y", types)
    elseif axes == "origin" then
      self:ClearPoints("originx", types)
      self:ClearPoints("originy", types)
    elseif axes == "all" then
      self:ClearPoints("alignment", types)
      self:ClearPoints("origin", types)
    else
      self.__axes[axes]:ClearPoints(types)
    end
  end
  
  -- ffs stop removing this, zorba, you're an idiot
  -- The purpose of this is so we don't send clicks to a window that hasn't been fully constructed
  -- "Exists" means that it has a point on either an X or a Y axis.
  -- There. Done.
  function Region_Type:Exists()
    return self.__axes.x.exppoint > 0 and self.__axes.y.exppoint > 0
  end
  
  function Region_Type:SetWidth(width)
    self.__axes.x:Set("size", {}, function () return width end)
  end
  function Region_Type:SetHeight(height)
    self.__axes.y:Set("size", {}, function () return height end)
  end
  function Region_Type:SetSize(width, height)
    if not height then height = width end
    self:SetWidth(width)
    self:SetHeight(height)
  end
  
  function Region_Type:GetWidth()
    return math.abs(self.__width:Get())
  end
  function Region_Type:GetHeight()
    return math.abs(self.__height:Get())
  end
  function Region_Type:GetSize()
    return self:GetWidth(), self:GetHeight()
  end
  
  function Region_Type:GetLeft()
    return self.__point_left:Get()
  end
  function Region_Type:GetRight()
    return self.__point_right:Get()
  end
  function Region_Type:GetTop()
    return self.__point_top:Get()
  end
  function Region_Type:GetBottom()
    return self.__point_bottom:Get()
  end
  
  function Region_Type:GetXCenter()
    return self.__axes.x:Get(0.5):Get()
  end
  function Region_Type:GetYCenter()
    return self.__axes.y:Get(0.5):Get()
  end
  function Region_Type:GetCenter()
    return self:GetXCenter(), self:GetYCenter()
  end

  function Region_Type:GetBounds()
    return self:GetLeft(), self:GetTop(), self:GetRight(), self:GetBottom()
  end

  function Region_Type:SetParent(parent)
    self:Detach()
    
    self.parent = parent
    
    if parent then
      if not parent.children then parent.children = {} end
      table.insert(parent.children, self)
      
      parent:resort_children()
    end
    
    self.__parent_node:Invalidate()
  end
  function Region_Type:Detach()
    if self.parent then
      for k, v in ipairs(self.parent.children) do
        if v == self then
          table.remove(self.parent.children, k)
          self.parent._update_children_done = false  -- we might need to reprocess
          break
        end
      end
    end
    self.parent = nil
    
    -- Note: Technically we're still anchored. This is A-OK, we might reattach to something else later on. If we get GC'ed, well, that's why the anchor children tables are weak-keyed
    
    -- How do we deal with the translation now?
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
    self._update_children_done = false -- we might need to reprocess
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
  
  function Region_Type:GetMouse()
    if not self.__convert_frame then self.__convert_frame = CreateFrame("Frame", self) end
    
    if not self.__convert_x then self.__convert_x = GetAxisConversion("x", UIParent, self.__convert_frame) end
    if not self.__convert_y then self.__convert_y = GetAxisConversion("x", UIParent, self.__convert_frame) end
    
    local mx, my = GetMouse() -- the global one, that is
    
    return mx * self.__convert_x:Get().scale + self.__convert_x:Get().offset, my * self.__convert_y:Get().scale + self.__convert_y:Get().offset
  end
  
  
  local function anchor_to_standard(self, selfaxis, dest, target, targetaxis, src, offset)
    local handle = target:GetHandle(targetaxis, src)
    local convert = GetAxisConversion(targetaxis, target, self)
    self:SetHandle(selfaxis, dest, {handle, convert}, function () return handle:Get() * convert:Get().scale + convert:Get().offset + offset end)
  end
  local function anchor_standard_to_origin(self, axis, dest, offset)
    self:SetHandle(axis, dest, {}, function () return offset end)
  end
  
  function Region_Type:SetAllPoints(target)
    target = target or self.parent
    assert(target)
    anchor_to_standard(self, "x", 0, target, "x", 0, 0)
    anchor_to_standard(self, "x", 1, target, "x", 1, 0)
    anchor_to_standard(self, "y", 0, target, "y", 0, 0)
    anchor_to_standard(self, "y", 1, target, "y", 1, 0)
    -- grunch
  end
  
  local strconv = {
    TOPLEFT = {0, 0},
    TOPRIGHT = {1, 0},
    BOTTOMLEFT = {0, 1},
    BOTTOMRIGHT = {1, 1},
    CENTER = {0.5, 0.5},
    CENTERX = {0.5, nil},
    CENTERY = {nil, 0.5},
    LEFT = {0, nil},
    RIGHT = {1, nil},
    TOP = {nil, 0},
    BOTTOM = {nil, 1},
  }
  function Region_Type:SetPoint(a, b, c, d, e, f, g, h)
    local origin = false
    if a == "origin" then
      origin = true
      a, b, c, d, e, f, g = b, c, d, e, f, g, h
      
      if not (type(b) == "number" or b == nil) then
        --the next values aren't coordinates, so we're just setting origin 0,0
        a, b, c, d, e, f, g, h = 0, 0, a, b, c, d, e, f
      end
    elseif type(a) == "string" then
      if not strconv[a] then
        assert(false)
      end
      
      a, b, c, d, e, f, g, h = strconv[a][1], strconv[a][2], b, c, d, e, f, g
    end
    
    if c == "origin" then
      local sx, sy, _, ox, oy = a, b, c, d, e
      if sx then
        anchor_standard_to_origin(self, "x", sx, ox or 0)
      end
      if sy then
        anchor_standard_to_origin(self, "y", sy, oy or 0)
      end
      
      return
    end
    
    if type(d) == "string" then
      if not strconv[d] then
        assert(false)
      end
      
      a, b, c, d, e, f, g, h = a, b, c, strconv[d][1], strconv[d][2], e, f, g
    end
    
    local sx, sy, target, ex, ey, ofsx, ofsy = a, b, c, d, e, f, g
    if sx and ex then
      anchor_to_standard(self, origin and "originx" or "x", sx, target, "x", ex, ofsx or 0)
    end
    if sy and ey then
      anchor_to_standard(self, origin and "originy" or "y", sy, target, "y", ey, ofsy or 0)
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
  
  --[[
  function Region_Type:SetCoordinateScale(x, y, scale, rotate)
    assert((x and y and scale) or not (x or y or scale))
    self.cs_x, self.cs_y, self.cs_scale, self.cs_rotate = x, y, scale, rotate
  end]]
  
  
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
    
    local pushed
    local ox, oy, sx, sy = self.__origin_x:Get(), self.__origin_y:Get(), self.__origin_x_scale:Get(), self.__origin_y_scale:Get()
    if ox ~= 0 or oy ~= 0 or sx ~= 1 or sy ~= 1 then
      pushed = gl.Get("PROJECTION_MATRIX")
      gl.MatrixMode("PROJECTION")
      gl.Translate(ox, oy, 0)
      gl.Scale(sx, sy, 0)
    end
    
    if self.Draw then self:Draw() end
    
    if self.children then
      for _, k in ipairs(self.children) do
        k:Render()
      end
    end
    
    if pushed then
      gl.MatrixMode("PROJECTION")
      gl.LoadMatrix(pushed)
    end
  end
  
  function Region_Type:Update(quanta)
    if self._last_updated ~= quanta then
      self._last_updated = quanta
      self._update_children_done = false
      if self.Tick then self:Tick() end
    end
    
    while not self._update_children_done do
      self._update_children_done = true
      
      if self.children then
        for _, k in ipairs(self.children) do
          k:Update(quanta)
        end
      end
    end
  end
  
  --function Region_Type_mt:__newindex(k, v)
    --print(k, v)
    --rawset(self, k, v)
  --end
  
  local globid = 0
  function Region(parent, name)
    local reg = setmetatable({}, Region_Type_mt)
    if not parent then parent = UIParent end
    
    reg.__globid = globid
    globid = globid + 1
    assert(globid ~= globid + 1) -- double bounds check, should never be hit
    
    reg.__name = "id" .. globid .. " " .. name
    reg.__axes = {}
    
    reg.__axes.x = CreateAxis(reg.__name .. " x", 40)
    reg.__axes.y = CreateAxis(reg.__name .. " y", 40)
    reg.__axes.originx = CreateAxis(reg.__name .. " originx", 1)
    reg.__axes.originy = CreateAxis(reg.__name .. " originy", 1)
    
    reg.__point_left = reg.__axes.x:Get(0)
    reg.__point_right = reg.__axes.x:Get(1)
    reg.__point_top = reg.__axes.y:Get(0)
    reg.__point_bottom = reg.__axes.y:Get(1)
    reg.__width = reg.__axes.x:Get("size")
    reg.__height = reg.__axes.y:Get("size")
    
    reg.__origin_x = reg.__axes.originx:Get(0)
    reg.__origin_y = reg.__axes.originy:Get(0)
    reg.__origin_x_scale = reg.__axes.originx:Get("size")
    reg.__origin_y_scale = reg.__axes.originy:Get("size")
    
    reg.__parent_node = CreateNode(reg.__name .. " parent")
    reg.__parent_node:Set({}, function () return reg.parent end)
    
    if parent then reg:SetParent(parent) end
    return reg
  end
end

-- init UIParent
do
  assert(not UIParent)
  UIParent = Region(nil, "UIParent")
  
  UIParent:SetPoint("TOPLEFT", "origin", 0, 0)
  UIParent:SetPoint("BOTTOMRIGHT", "origin", GetScreenX(), GetScreenY())
end

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

function TraverseUp(button)
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
  
  local rv = TraverseUpWorker(UIParent, x, y, false)
  
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
  
  local frame_target = TraverseUp(button)
  -- Otherwise, we first figure out what the topmost frame it's on is, with topmost being simply defined as "last in the render order but with the key over it".
  -- Then we traverse up, looking for something that wants to eat it.
  while frame_target do
    if frame_target.Key then
      passdown = frame_target:Key(button, ascii, event)
      if not passdown then return end
    end
    frame_target = frame_target.parent
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
