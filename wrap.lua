
function core__loadfile(filename)
  local rv, err = xpcall(function () local tv, err = loadfile(filename) if not tv then print(err) error(err) else tv() end end, function (ter) return ter .. "\n" .. debug.traceback() end)
  if err then error(err) else return end
end

function loop_wrap(...)
  local dt = {...}
  local it = select('#', ...)
  local rv, err = xpcall(function () return loop(unpack(dt, 1, it)) end, function (ter) return ter .. "\n" .. debug.traceback() end)
  if err then error(err) else return rv end
end


local crc = coroutine.create
function coroutine.create(cof, ...)
  local dt = {...}
  local it = select('#', ...)
  return crc(function ()
    local rv, err = xpcall(function () return cof(unpack(dt, 1, it)) end, function (ter) return ter .. "\n" .. debug.traceback() end)
    if err then error(err) else return rv end
  end)
end
