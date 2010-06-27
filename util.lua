
local mode, platform = ...

math.randomseed(os.time())

function rotate_mod(x, m)
  while x <= 0 do x = x + m end
  while x > m do x = x - m end
  return x
end

function export_items_rw(tab, items)
  local lookup = {}
  for _, v in pairs(items) do
    lookup[v] = true
  end
  
  return setmetatable({}, {__index = function(_, k) return lookup[k] and tab[k] or nil end, __newindex = function(t, k, a) if lookup[k] then tab[k] = a else t[k] = a end end})
end

function export_items_ro(tab, items)
  local lookup = {}
  for _, v in pairs(items) do
    if not tab[v] then
      print("Can't find", v)
      assert(tab[v], v)
    end
    lookup[v] = tab[v]
  end
  
  return setmetatable({}, {__index = lookup})
end


function perfbar(r, g, b, func, ...)
  local pb = Perfbar_Init(r, g, b)
  func(...)
  pb:Destroy()
end



function io.dump(filename, contents)
  t = io.open(filename, "wb")
  t:write(contents)
  t:close()
end
function io.snatch(filename)
  t = io.open(filename, "rb")
  if not t then t = io.open("data/" .. filename, "rb") end
  if not t then return end
  local dat = t:read("*all")
  t:close()
  return dat
end


function GetMouse() return GetMouseX(), GetMouseY() end

function approach(current, target, delta)
  if math.abs(current - target) <= delta then
    return target
  end
  
  if target > current then
    return current + delta
  else
    return current - delta
  end
end
function clamp(cur, min, max)
  if cur < min then return min end
  if cur > max then return max end
  return cur
end
function lerp(d, s, e)
  return s * (1 - d) + e * d
end
function bezier(x0, x1, x2, x3, t)
  local cx = 3 * (x1 - x0)
  local bx = 3 * (x2 - x1) - cx
  local ax = x3 - x0 - cx - bx
  return ax * t * t * t + bx * t * t + cx * t + x0
end
function cubic(x0, x1, x2, x3, t)
  local a0 = x3 - x2 - x0 + x1
  local a1 = x0 - x1 - a0
  local a2 = x2 - x0
  local a3 = x1
  
  return a0 * t * t * t + a1 * t * t + a2 * t + a3
end

function PlaySound(snd, vol)
  PlaySound_Core(snd, vol or 1)
end
function ControlSound(snd, vol, loop)
  return ControlSound_Core(snd, vol or 1, loop or false)
end

glutil = {}
function glutil.SetScreen(sx, sy, ex, ey)
  local ax = (sx + ex) / 2
  local ay = (sy + ey) / 2
  
  local dx = ex - sx
  local dy = ey - sy
  
  gl.Disable("CULL_FACE")
  
  gl.MatrixMode("PROJECTION")
  gl.LoadIdentity()
  gl.Ortho(sx, ex, ey, sy, -100, 100)
  
  gl.MatrixMode("MODELVIEW")
  gl.LoadIdentity()
  
  --gl.Translate(-ax, -ay, 0)
end
function glutil.SetScreenCentered(x, y, pixelgrid)
  glutil.SetScreen(x - 512 / pixelgrid, y - 384 / pixelgrid, x + 512 / pixelgrid, y + 384 / pixelgrid)
end

function glutil.ResetScreen()
  glutil.SetScreen(0, 0, 1024, 768)
end


function glutil.RenderCenteredSprite(tex, x, y, width, height, r, g, b, a)
  glutil.RenderBoundedSprite(tex, x - width / 2, y - height / 2, x + width / 2, y + height / 2, r, g, b, a)
end

function glutil.RenderBoundedSprite(tex, sx, sy, ex, ey, r, g, b, a)
  if type(sx) == "table" then
    return glutil.RenderBoundedSprite(tex, sx[1], sx[2], sx[3], sx[4], sy, ex, ey, r)
  end
  
  --sx, sy, ex, ey = math.floor(sx + 0.5) + 0.375, math.floor(sy + 0.5) + 0.375, math.floor(ex + 0.5) + 0.375, math.floor(ey + 0.5) + 0.375
  
  assert(sx and sy and ex and ey)
  local teex = tex:GetWidth() / tex:GetInternalWidth()
  local teey = tex:GetHeight() / tex:GetInternalHeight()
  assert(teex and teey)

  
  
  local xadj = 1 / tex:GetInternalWidth() / 2
  local yadj = 1 / tex:GetInternalHeight() / 2
  
  tex:SetTexture()
  gl.Color(r or 1, g or 1, b or 1, a or 1)
  glutil.RenderArray("TRIANGLE_FAN", 2, {sx, sy, ex, sy, ex, ey, sx, ey}, nil, nil, 2, {0 + xadj, 0 + yadj, teex + xadj, 0 + yadj, teex + xadj, teey + yadj, 0 + xadj, teey + yadj})
  SetNoTexture()
end

function glutil.RenderBoundedSpriteSegment(tex, sx, sy, ex, ey, ttsx, ttsy, ttex, ttey)
  assert(sx and sy and ex and ey)
  local teex = tex:GetWidth() / tex:GetInternalWidth()
  local teey = tex:GetHeight() / tex:GetInternalHeight()
  assert(teex and teey)
  
  local xadj = 1 / tex:GetInternalWidth() / 2
  local yadj = 1 / tex:GetInternalHeight() / 2
  
  tex:SetTexture()
  gl.Color(1, 1, 1)
  glutil.RenderArray("TRIANGLE_FAN", 2, {sx, sy, ex, sy, ex, ey, sx, ey}, nil, nil, 2, {teex * ttsx, teey * ttsy, teex * ttex, teey * ttsy, teex * ttex, teey * ttey, teex * ttsx, teey * ttey})
  SetNoTexture()
end

function glutil.RenderCenteredBox(r, g, b, x, y, width, height)
  glutil.RenderBoundedBox(r, g, b, x - width / 2, y - height / 2, x + width / 2, y + height / 2)
end

function glutil.RenderBoundedBox(r, g, b, sx, sy, ex, ey)
  gl.Color(r, g, b)
  
  glutil.RenderArray("TRIANGLE_FAN", 2, {sx, sy, ex, sy, ex, ey, sx, ey})
end

function glutil.RenderCenteredEmptyBox(r, g, b, x, y, width, height)
  glutil.RenderEmptyBox(r, g, b, x - width / 2, y - height / 2, x + width / 2, y + height / 2)
end

function glutil.RenderEmptyBox(r, g, b, sx, sy, ex, ey)
  gl.Color(r, g, b)
  glutil.RenderArray("LINE_LOOP", 2, {sx, sy, ex, sy, ex, ey, sx, ey})
end


function coroutine.pause(frames)
  for i = 1, frames do coroutine.yield() end
end


local Tex = Texture
local texcache = {}
function Texture(...)
  for i = 1, select('#', ...) do
    local texname = select(i, ...)
    if texcache[texname] then return texcache[texname] end
    
    texcache[texname] = Tex(texname)
    if texcache[texname] then return texcache[texname] end
  end
  
  print("No such texture list", ...)
  assert(false)
end

function sign(x)
  if x == 0 then return 0 end
  if x < 0 then return -1 end
  if x > 0 then return 1 end
  assert(false)
end




--print(gl.Get("MAX_PROJECTION_STACK_DEPTH"))

--local log = io.open("gl.txt", "w")

-- hurrr
if true then
  local shutup = false
  function testerror(bef, nam)
    if shutup then return end
    local err = gl.GetError()
    if err ~= "NO_ERROR" then
      print("GL ERROR: ", err, bef, nam)
      if mode then assert(err == "NO_ERROR", err ..  "   " .. bef .. " " .. nam) end -- fuckyou
    end
  end
  for k, v in pairs(gl) do
    local tk = k
    if tk ~= "GetError" then
      gl[tk] = function (...)
        testerror("before", k)
        return (function (...)
          if tk == "Begin" then
            assert(not shutup)
            shutup = true
          elseif tk == "End" then
            assert(shutup)
            shutup = false
          end
          testerror("after", k)
          return ...
        end)(v(...))
      end
    end
  end
end

--[[
for k, v in pairs(gl) do
  gl[k] = function (...)
    print("before", k)
    return (function (...)
      print("after", k)
      return ...
    end)(v(...))
  end
end
]]



do
  local List_params = {}
  
  function List_params:Create()
    gl.NewList(self.listid:get(), "COMPILE")
  end
  function List_params:End()
    gl.EndList()
  end
  
  function List_params:Call()
    gl.CallList(self.listid:get())
  end
  -- we don't yet support deleting
  
  function glutil.List()
    local ite = setmetatable({}, {__index = List_params})
    ite.listid = GlListID()
    
    return ite
  end
end
function glutil.Autolist(target, name, process)
  if not target[name] then
    target[name] = glutil.List()
    target[name]:Create()
    
    process()
    
    target[name]:End()
  end
  
  target[name]:Call()
end

function glutil.Shader(typ, program)
  local shader = {id = GlShader(typ .. "_SHADER")}
  glutil.ShaderSource(shader, program)
  glutil.CompileShader(shader)
  return shader
end

function glutil.Program()
  local prog = {id = GlProgram()}
  return prog
end

function glutil.Framebuffer()
  local prog = {id = GlFramebuffer()}
  return prog
end

function glutil.Renderbuffer()
  local prog = {id = GlRenderbuffer()}
  return prog
end

function glutil.Texture()
  local prog = {id = GlTexture()}
  return prog
end

print(platform)
if platform ~= "iphone" and platform ~= "iphone_sim" then
  local snatch = {
    ShaderSource = true,
    CompileShader = true,
    AttachShader = true,
    LinkProgram = true,
    UseProgram = true,
    
    GetShaderInfoLog = true,
    GetProgramInfoLog = true,
    GetShader = true,
    GetProgram = true,
    
    UniformI = true,
    UniformF = true,
    GetUniformLocation = true,
    
    --UniformI = true,
    VertexAttrib = true,
    GetAttribLocation = true,
    
    BindFramebuffer = true,
    BindRenderbuffer = true,
    FramebufferRenderbuffer = true,
    FramebufferTexture2D = true,
    
    BindTexture = true,
  }
  for k in pairs(snatch) do
    snatch[k] = gl[k]
    assert(snatch[k], k)
  end

  local attrib_lookup = {}
  local uniform_lookup = {}

  function glutil.ShaderSource(shader, source)
    snatch.ShaderSource(shader.id:get(), {source})
  end
  function glutil.CompileShader(shader)
    snatch.CompileShader(shader.id:get())
    local infolog = snatch.GetShaderInfoLog(shader.id:get())
    print(infolog)
    print(mode)
    if (infolog ~= "" and mode) or glutil.GetShader(shader, "COMPILE_STATUS") ~= "TRUE" then
      print("Failed to build shader", glutil.GetShader(shader, "COMPILE_STATUS"))
      assert(false)
    end
  end
  function glutil.AttachShader(program, shader)
    snatch.AttachShader(program.id:get(), shader.id:get())
  end
  function glutil.LinkProgram(program)
    snatch.LinkProgram(program.id:get())
    attrib_lookup[program.id:get()] = nil
    uniform_lookup[program.id:get()] = nil
  end
  function glutil.UseProgram(program)
    snatch.UseProgram(program and program.id:get() or 0)
  end

  function glutil.GetShader(shader, flag)
    return snatch.GetShader(shader.id:get(), flag)
  end
  function glutil.GetProgram(shader, flag)
    return snatch.GetProgram(shader.id:get(), flag)
  end

  function glutil.UniformI(program, text, ...)
    local pid = program.id:get()
    if not uniform_lookup[pid] then uniform_lookup[pid] = {} end
    local loca = uniform_lookup[pid][text]
    if not loca then
      uniform_lookup[pid][text] = snatch.GetUniformLocation(pid, text)
      loca = uniform_lookup[pid][text]
    end
    
    if loca == -1 then
      print("WARNING: Uniform " .. text .. " is unused")
    else
      snatch.UniformI(loca, ...)
    end
  end
  function glutil.UniformF(program, text, ...)
    local pid = program.id:get()
    if not uniform_lookup[pid] then uniform_lookup[pid] = {} end
    local loca = uniform_lookup[pid][text]
    if not loca then
      uniform_lookup[pid][text] = snatch.GetUniformLocation(pid, text)
      loca = uniform_lookup[pid][text]
    end
    
    if loca == -1 then
      print("WARNING: Uniform " .. text .. " is unused")
    else
      snatch.UniformF(loca, ...)
    end
  end

  function glutil.VertexAttribInit(program, text)
    local pid = program.id:get()
    if not attrib_lookup[pid] then attrib_lookup[pid] = {} end
    local loca = snatch.GetAttribLocation(pid, text)
    assert(loca)
    print("ALOOKUP", pid, text, loca)
    attrib_lookup[pid][text] = loca
  end
  --[[function glutil.VertexAttribI(program, text, ...)
    local pid = program.id:get()
    assert(attrib_lookup[pid] and attrib_lookup[pid][text])
    snatch.VertexAttribI(attrib_lookup[pid][text], ...)
  end]]
  function glutil.VertexAttrib(program, text, ...)
    local pid = program.id:get()
    assert(attrib_lookup[pid] and attrib_lookup[pid][text])
    if attrib_lookup[pid][text] == -1 then
      print("WARNING: Attribute " .. text .. " is unused", pid)
    else
      snatch.VertexAttrib(attrib_lookup[pid][text], ...)
    end
  end

  local lastfb
  function glutil.BindFramebuffer(target, framebuffer)
    lastfb = framebuffer
    if framebuffer then
      snatch.BindFramebuffer(target, framebuffer.id:get())
    else
      snatch.BindFramebuffer(target, 0)
    end
  end
  function glutil.BindRenderbuffer(target, renderbuffer)
    if renderbuffer then
      snatch.BindRenderbuffer(target, renderbuffer.id:get())
    else
      snatch.BindRenderbuffer(target, 0)
    end
  end
  function glutil.FramebufferRenderbuffer(target, attach, rbt, rb)
    snatch.FramebufferRenderbuffer(target, attach, rbt, rb.id:get())
  end
  function glutil.FramebufferTexture2D(target, attach, textarg, tex, level)
    lastfb[attach] = tex  -- we do this to keep it from being garbage collected
    snatch.FramebufferTexture2D(target, attach, textarg, tex.id:get(), level)
  end
  
  function glutil.BindTexture(typ, tex)
    if tex then
      snatch.BindTexture(typ, tex.id:get())
    else
      snatch.BindTexture(typ, 0)
    end
  end

  for k in pairs(snatch) do
    gl[k] = nil -- yoink
  end
end


function glutil.RenderArray(mode, vertex_size, vertices, color_size, color, texture_size, texture)
  gl.EnableClientState("VERTEX_ARRAY")
  gl.VertexPointer(vertex_size, "FLOAT", vertices)
  if color_size then
    assert(#color / color_size == #vertices / vertex_size)
    gl.EnableClientState("COLOR_ARRAY")
    gl.ColorPointer(color_size, "FLOAT", color)
  end
  if texture_size then
    assert(#texture / texture_size == #vertices / vertex_size)
    gl.EnableClientState("TEXTURE_COORD_ARRAY")
    gl.TexCoordPointer(texture_size, "FLOAT", texture)
  end
  
  gl.DrawArrays(mode, 0, #vertices / vertex_size)
  
  gl.DisableClientState("VERTEX_ARRAY")
  if color_size then
    gl.DisableClientState("COLOR_ARRAY")
  end
  if texture_size then
    gl.DisableClientState("TEXTURE_COORD_ARRAY")
  end
end


function math.round(x)
  return math.floor(x + 0.5)
end

function impose_standard_ai(target)
  assert(not target.Tick)
  local tick_ai_reservoir = {}
  local clears = {}
  local clear_count = 0
  function wipe_item(item)
    clears[item] = true
    clear_count = clear_count + 1
  end
  function target:TickStandard()
    for _, v in ipairs(tick_ai_reservoir) do
      v(self)
    end
    
    if clear_count > 0 then
      local ntai = {}
      for _, v in ipairs(tick_ai_reservoir) do
        if not clears[v] then
          table.insert(ntai, v)
        end
      end
      
      clears, clear_count = {}, 0
      tick_ai_reservoir = ntai
    end
  end
  if not target.Tick then target.Tick = target.TickStandard end
  function target:Tick_Add(func)
    table.insert(tick_ai_reservoir, func)
  end
  function target:Tick_AddCoro(func)
    local coro
    coro = coroutine.wrap(function ()
      func(self)
      
      wipe_item(coro)
    end)
    table.insert(tick_ai_reservoir, coro)
  end
end


local function heap_left(x) return (2*x) end
local function heap_right(x) return (2*x + 1) end
local function heap_sane(heap)
  local dmp = ""
  local finishbefore = 2
  for i = 1, #heap do
    if i == finishbefore then
      print(dmp)
      dmp = ""
      finishbefore = finishbefore * 2
    end
    dmp = dmp .. string.format("%f ", heap[i].c)
  end
  print(dmp)
  print("")
  for i = 1, #heap do
    --[[ assert(not heap[heap_left(i)] or heap[i].c <= heap[heap_left(i)].c) ]]
    --[[ assert(not heap[heap_right(i)] or heap[i].c <= heap[heap_right(i)].c) ]]
  end
end
function heap_insert(heap, item)
  --[[ assert(item) ]]
  table.insert(heap, item)
  local pt = #heap
  while pt > 1 do
    local ptd2 = math.floor(pt / 2)
    if heap[ptd2].c <= heap[pt].c then
      break
    end
    local tmp = heap[pt]
    heap[pt] = heap[ptd2]
    heap[ptd2] = tmp
    pt = ptd2
  end
  --heap_sane(heap)
end
function heap_extract(heap)
  local rv = heap[1]
  if #heap == 1 then table.remove(heap) return rv end
  heap[1] = table.remove(heap)
  local idx = 1
  while idx < #heap do
    local minix = idx
    --local hl, hr = heap_left(idx), heap_right(idx)
    local hl, hr = 2*idx, 2*idx+1 -- these had better be equivalent to the line above one
    if heap[hl] and heap[hl].c < heap[minix].c then minix = hl end
    if heap[hr] and heap[hr].c < heap[minix].c then minix = hr end
    if minix ~= idx then
      local tx = heap[minix]
      heap[minix] = heap[idx]
      heap[idx] = tx
      idx = minix
    else
      break
    end
  end
  --heap_sane(heap)
  return rv
end




do
  local sms = ShowMouseCursor
  local shown = true
  function ShowMouseCursor(flag)
    sms(flag)
    shown = flag
  end
  function MouseCursorShown()
    return shown
  end
end