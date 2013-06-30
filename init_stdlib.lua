assert(loadfile("glorp/init_stdlib_coro.lua"))()

local tps = 60

local tick = External.Command.Event.Create(External, "System.Tick")
local lasttick = External.Inspect.System.Time.Real()
External.Event.System.Update.Begin:Attach(function ()
  local ticksdone = 0
  while lasttick + 1/tps < External.Inspect.System.Time.Real() do
    ticksdone = ticksdone + 1
    lasttick = lasttick + 1/tps
    tick()
    if ticksdone > 3 then
      -- ran out of frames, kill kill kill
      lasttick = External.Inspect.System.Time.Real()
    end
  end
end)

External.Command.Environment.Insert(External, "Utility.TicksFromSeconds", function (s)
  return s * tps
end)

-- must be last
assert(loadfile("glorp/init_stdlib_environment.lua"))()
