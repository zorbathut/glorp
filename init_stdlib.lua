assert(loadfile("glorp/init_stdlib_coro.lua"))()

local tick = External.Command.Event.Create(External, "System.Tick")
local lasttick = External.Inspect.System.Time.Real()
External.Event.System.Update.Begin:Attach(function ()
  local ticksdone = 0
  while lasttick + 1/60 < External.Inspect.System.Time.Real() do
    ticksdone = ticksdone + 1
    lasttick = lasttick + 1/60
    tick()
    if ticksdone > 3 then
      -- ran out of frames, kill kill kill
      lasttick = External.Inspect.System.Time.Real()
    end
  end
end)

-- must be last
assert(loadfile("glorp/init_stdlib_environment.lua"))()
