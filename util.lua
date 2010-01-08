
math.randomseed(os.time())

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
  gl.Ortho(sx, ex, ey, sy, -1, 1)
  --gl.Ortho(-dx / 2, dx / 2, dy / 2, -dy / 2, -1, 1)
  
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
  gl.Begin("QUADS")
  gl.TexCoord(0 + xadj, 0 + yadj)
  gl.Vertex(sx, sy)
  gl.TexCoord(teex + xadj, 0 + yadj)
  gl.Vertex(ex, sy)
  gl.TexCoord(teex + xadj, teey + yadj)
  gl.Vertex(ex, ey)
  gl.TexCoord(0 + xadj, teey + yadj)
  gl.Vertex(sx, ey)
  gl.End()
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
  gl.Begin("QUADS")
  gl.TexCoord(teex * ttsx, teey * ttsy)
  gl.Vertex(sx, sy)
  gl.TexCoord(teex * ttex, teey * ttsy)
  gl.Vertex(ex, sy)
  gl.TexCoord(teex * ttex, teey * ttey)
  gl.Vertex(ex, ey)
  gl.TexCoord(teex * ttsx, teey * ttey)
  gl.Vertex(sx, ey)
  gl.End()
  SetNoTexture()
end

function glutil.RenderCenteredBox(r, g, b, x, y, width, height)
  glutil.RenderBoundedBox(r, g, b, x - width / 2, y - height / 2, x + width / 2, y + height / 2)
end

function glutil.RenderBoundedBox(r, g, b, sx, sy, ex, ey)
  gl.Color(r, g, b)
  
  gl.Begin("QUADS")
  gl.Vertex(sx, sy)
  gl.Vertex(ex, sy)
  gl.Vertex(ex, ey)
  gl.Vertex(sx, ey)
  gl.End()
end

function glutil.RenderCenteredEmptyBox(r, g, b, x, y, width, height)
  glutil.RenderEmptyBox(r, g, b, x - width / 2, y - height / 2, x + width / 2, y + height / 2)
end

function glutil.RenderEmptyBox(r, g, b, sx, sy, ex, ey)
  gl.Color(r, g, b)
  
  gl.Begin("LINE_LOOP")
  gl.Vertex(sx, sy)
  gl.Vertex(ex, sy)
  gl.Vertex(ex, ey)
  gl.Vertex(sx, ey)
  gl.End()
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
  
  function gl.List()
    local ite = setmetatable({}, {__index = List_params})
    ite.listid = GlListID()
    
    return ite
  end
end

function gl.Shader(typ, program)
  local shader = {id = GlShader(typ .. "_SHADER")}
  gl.ShaderSource(shader, program)
  gl.CompileShader(shader)
  return shader
end

function gl.Program()
  local prog = {id = GlProgram()}
  return prog
end

local wrap_vals = {
  ShaderSource = {true},
  CompileShader = {true},
  AttachShader = {true, true},
  LinkProgram = {true},
  UseProgram = {true},
}

local function strip(_, ...) return ... end
local function reco_proc(nam, flags, ofs, ...)
  if select("#", ...) == 0 then return end
  
  local tite = select(1, ...)
  
  if flags[ofs] then
    if type(tite) ~= "table" or not tite.id then
      error(("Couldn't remap ID for function %s, parameter %d (%s)"):format(nam, ofs, tostring(tite)))
    end
    tite = tite.id:get()
  end
  
  return tite, reco_proc(nam, flags, ofs + 1, strip(...))
end
for k, v in pairs(gl) do
  if wrap_vals[k] then
    gl[k] = function(...)
      return v(reco_proc(k, wrap_vals[k], 1, ...))
    end
  end
end
