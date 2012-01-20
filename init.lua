
loadfile("glorp/init_util.lua")()
loadfile("glorp/init_event.lua")()

function InitComplete()
  dump(External.Event)
  table.insert(External.Event.System.Update.Begin)
end
