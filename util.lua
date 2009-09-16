
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

glutil = {}
function glutil.SetScreen(sx, sy, ex, ey)
  gl.MatrixMode("PROJECTION")
  gl.LoadIdentity()
  gl.Ortho(sx, ex, ey, sy, -1, 1)
  
  gl.MatrixMode("MODELVIEW")
  gl.LoadIdentity()
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
  local teex = tex:GetWidth() / tex:GetInternalWidth()
  local teey = tex:GetHeight() / tex:GetInternalHeight()
  
  tex:SetTexture()
  gl.Color(r or 1, g or 1, b or 1, a or 1)
  gl.Begin("QUADS")
  gl.TexCoord(0, 0)
  gl.Vertex(sx, sy)
  gl.TexCoord(teex, 0)
  gl.Vertex(ex, sy)
  gl.TexCoord(teex, teey)
  gl.Vertex(ex, ey)
  gl.TexCoord(0, teey)
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

