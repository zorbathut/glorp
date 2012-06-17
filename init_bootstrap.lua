-- Bootstrap functionality that is required to get the basic error handling working.

-- pack/unpack, remove if luajit supports these
if not table.pack then
  function table.pack(...)
    local t = {...}
    t.n = select("#", ...)
    return t
  end
  function table.unpack(t)
    return unpack(t, 1, t.n)
  end
end

-- split into lines, remove various cruft from the error dumps
local function stripTraceback(err)
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

local function displayError(err)
  -- first unroll all the recursive elements we may have
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
    chunkies[k] = stripTraceback(chunkies[k])
  end
  
  -- now we finally have a set of reasonable coroutine error blocks
  local rv = "\n\n\n" .. chunkies[1] .. "\n\nStacktrace:\n"
  for k = 2, #chunkies do
    if k ~= 2 then
      rv = rv .. "  ---- Coroutine boundary\n"
    end
    rv = rv .. chunkies[k]
  end

  print(rv .. "\n")
  error(rv .. "\n")
end

local function wrap(target, ...)
  assert(target)
  local dt = table.pack(...)
  local rv, err = xpcall(function () return table.pack(target(table.unpack(dt))) end, function (ter) return {ter, debug.traceback()} end)
  if not rv then
    displayError(err)
    return
  else
    return table.unpack(err)
  end
end

local lf = loadfile
function loadfile(file)
  local dat, rv = lf("data/" .. file)

  if rv and rv:find("No such file or directory") then
    dat, rv = lf(file)
  end

  return dat, rv
end

local setup_env = {}
for k, v in pairs(_G) do
  setup_env[k] = v
end
setup_env._G = setup_env
setup_env.External = _G
setup_env.loadfile = function (param)
  local func, rv = loadfile(param)
  if func then func = setfenv(func, setup_env) end
  return func, rv
end
setup_env.params = ...
setup_env.Wrap = wrap

-- kick off the rest of the init
wrap(function ()
  assert(setup_env.loadfile("glorp/init.lua"))()
end)

return setup_env
