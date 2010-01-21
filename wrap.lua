package.path = package.path .. ";data/?.lua;glorp/resources/?.lua"

collectgarbage("stop")

--[[
local ass = assert
function assert(parm, ...)
  if not parm then
    print(...)
    ass(false)
  end
end]]

do
  local jo = require("jit.opt")
  if jo then jo:start() end
end

local function barf(err)
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
  assert(target)
  local dt = {...}
  local it = select('#', ...)
  local rv, err = xpcall(function () return target(unpack(dt, 1, it)) end, function (ter) return {ter, debugstack_annotated()} end)
  if not rv then
    print("gwrap failure")
    barf(err)
  else
    return err
  end
end

function gcstep()
  perfbar(0.5, 0.5, 0.5, collectgarbage, "step", 1)
  --if rv then print("cycle") end
end

--[[
local crc = coroutine.create
function coroutine.create(cof, ...)
  local dt = {...}
  local it = select('#', ...)
  return crc(function ()
    local rv, err = xpcall(function () return cof(unpack(dt, 1, it)) end, function (ter) return ter .. "\n" .. debugstack_annotated() end)
    if err then error(err) else return rv end
  end)
end]]

local crw = coroutine.wrap
function coroutine.wrap(cof, ...)
  assert(cof)
  local dt = {...}
  local it = select('#', ...)
  
  return crw(function ()
    local rv, err = xpcall(function () return cof(unpack(dt, 1, it)) end, function (ter) return {ter, debugstack_annotated()} end)
    if not rv then
      error(err)
    else
      return err
    end
  end)
end




function fuckshit()
  if failover then return failover() end
end
function de_fuckshit(token)
  if de_failover then return de_failover(token) end
end