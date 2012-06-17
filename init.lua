-- Lua utilities and functionality. Note that this file is called from init_bootstrap.

assert(loadfile("glorp/init_util.lua"))()
assert(loadfile("glorp/init_event.lua"))()

function InitComplete()
  -- Global lua libraries that aren't needed for early init
  
  -- Must be after globals are introduced, but before locals are introduced
  assert(loadfile("glorp/init_environment.lua"))()
  
  -- Local utilities and tools, like console
  assert(loadfile("glorp/init_console.lua"))()
  
  -- All systems initialized, load the game
  local init = assert(loadfile("init.lua"))
  setfenv(init, External)()
end

-- Returns back to C++. InitComplete() will be called once the C++ side's initialization is done.