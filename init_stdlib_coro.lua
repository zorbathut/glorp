
-- Plays an arbitrary routine
InsertItem(External, "Command.Coro.Play", function (func)
  local coro = coroutine.create(func)
  assert(coroutine.resume(coro))  -- run it one cycle immediately
  
  local eventtable = getfenv(2).Event.System.Update.Begin
  
  local function runfunc()
    if coroutine.status(coro) == "dead" then
      eventtable:Detach(runfunc)
    else
      assert(coroutine.resume(coro))
    end
  end
  
  eventtable:Attach(runfunc)
end)

-- Pauses for a period of realtime
-- Not really ideal for game logic
InsertItem(External, "Command.Coro.Wait", function (duration)
  local stt = External.Inspect.System.Time.Real()
  
  while stt + duration > External.Inspect.System.Time.Real() do
    coroutine.yield()
  end
end)
