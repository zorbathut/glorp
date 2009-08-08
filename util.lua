

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
