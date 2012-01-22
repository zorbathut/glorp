
loadfile("glorp/init_util.lua")()
loadfile("glorp/init_event.lua")()

function InitComplete()
  dump(External.Event)
end
