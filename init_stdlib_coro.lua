
-- Pauses for a period of realtime
-- Not really ideal for game logic
InsertItem(External, "Command.Coro.Wait", function (duration)
  local stt = External.Inspect.System.Time.Real()
  
  while stt + duration > External.Inspect.System.Time.Real() do
    coroutine.yield()
  end
end)

function External.coroutine.spawn(coro, ...)
  assert(coro)
  local dt = table.pack(...)
  
  local result = coroutine.wrap(function ()
    local rv, err = xpcall(function () return table.pack(coro(table.unpack(dt))) end, function (ter) return {ter, debug.traceback()} end)
    if not rv then
      error(string.format("%s\n%s", err[1], err[2]))
    else
      coroutine.yield(table.unpack(err))
    end
  end)
  result()
  return result
end
