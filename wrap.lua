
function core__loadfile(filename)
  local rv, err = xpcall(function () local tv, err = loadfile(filename) if not tv then print(err) error(err) else tv() end end, function (ter) return ter .. "\n" .. debug.traceback() end)
  if err then error(err) else return end
end

local errorcount = 0
function strip_traceback(err)
  local linz = {}
  for i in err:gmatch("\n?([^\n]+)") do
    table.insert(linz, i)
  end
  if linz[#linz]:find("glorp/wrap.lua") then table.remove(linz, #linz) end
  if linz[#linz]:find("in function 'xpcall'") then table.remove(linz, #linz) end
  if linz[#linz]:find("\(tail call\)") then table.remove(linz, #linz) end
  if linz[1]:find("stack traceback:") then table.remove(linz, 1) end
  if linz[1]:find("glorp/wrap.lua") then table.remove(linz, 1) end
  if linz[1]:find("in function 'error'") then table.remove(linz, 1) end
  
  local rz = linz[1]
  for v = 2, #linz do
    rz = rz .. "\n" .. linz[v]
  end
  
  return rz .. "\n"
end

function generic_wrap(target, ...)
  local dt = {...}
  local it = select('#', ...)
  local rv, err = xpcall(function () return target(unpack(dt, 1, it)) end, function (ter) return {ter, debug.traceback()} end)
  if err then
    local chunkies = {}
    function doit(err)
      if type(err) == "string" then
        table.insert(chunkies, err)
      else
        doit(err[1])
        table.insert(chunkies, err[2])
      end
    end
    doit(err)
    
    for k = 2, #chunkies do
      chunkies[k] = strip_traceback(chunkies[k])
    end
    
    local rv = "\n\n\n" .. chunkies[1] .. "\n\nStacktrace:\n"
    for k = 2, #chunkies do
      if k ~= 2 then
        rv = rv .. "  ---- Coroutine boundary\n"
      end
      rv = rv .. chunkies[k]
      
    end
    
    error(rv .. "\n")
  else
    return rv
  end
end

--[[
local crc = coroutine.create
function coroutine.create(cof, ...)
  local dt = {...}
  local it = select('#', ...)
  return crc(function ()
    local rv, err = xpcall(function () return cof(unpack(dt, 1, it)) end, function (ter) return ter .. "\n" .. debug.traceback() end)
    if err then error(err) else return rv end
  end)
end]]

local crw = coroutine.wrap
function coroutine.wrap(cof, ...)
  local dt = {...}
  local it = select('#', ...)
  
  return crw(function ()
    local rv, err = xpcall(function () return cof(unpack(dt, 1, it)) end, function (ter) return {ter, debug.traceback()} end)
    if err then
      error(err)
    else
      return rv
    end
  end)
end
