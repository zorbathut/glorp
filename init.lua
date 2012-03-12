
loadfile("glorp/init_util.lua")()
loadfile("glorp/init_event.lua")()

function InitComplete()
  dump(External.Event)
  
  print(External.Frames)
  dump(External.Frames)
  
  print(External.Frames.Text(External.Frames.Root))
end
