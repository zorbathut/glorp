
function runfile(file, global)
  local dat, rv = loadfile(file)
  
  if rv and rv:find("No such file or directory") then
    dat, rv = loadfile("data/" .. file)
  end
  
  if rv and rv:find("No such file or directory") then
    dat, rv = loadfile("glorp/" .. file)
  end
  
  if rv then
    print(rv)
    assert(false)
  end
  
  if global then
    setfenv(dat, global)
  end
  
  dat()
end

runfile("util.lua")
runfile("ui.lua")


local mainmenu
local runninggame

local function destroy_game()
  if not runninggame then return end
  runninggame.UIParent:Detach() -- yunk
  runninggame = nil
end
local function Handle(param)
  if not param then return end
  
  if param == "start_game" then
    destroy_game()
    runninggame = runuifile("main.lua")
    mainmenu.UIParent:Hide()
  elseif param == "exit_game" then
    destroy_game()
    mainmenu.UIParent:Show()
  elseif param == "exit" then
    TriggerExit()
  else
    assert(false)
  end
end


function runuifile(file)
  local env = {}
  for k, v in pairs(_G) do
    env[k] = v
  end
  
  env._G = _G
  
  local uip = CreateFrame("Frame")
  uip:SetAllPoints()
  env.UIParent = uip
  env.CreateFrame = function (type, parent) return CreateFrame(type, parent or uip) end
  env.GlorpController = Handle
  env.loadfile = function (...)
    local dat, rv = loadfile(...)
    if dat then setfenv(dat, env) end
    return dat, rv
  end
  
  env.loop_tick = nil
  env.loop = nil
  env.render = nil
  env.key = nil
  
  runfile(file, env)
  
  return env
end

function stdwrap(token, ...)
  local context = runninggame or mainmenu
  
  if context[token] then
    return context[token](...)
  end
end

mainmenu = runuifile("menu_core.lua")

function tick_loop(...)
  stdwrap("tick_loop", ...)
end
function loop(...)
  stdwrap("loop", ...)
end
function render(...)
  stdwrap("render", ...)
  local context = runninggame or mainmenu
  context.UIParent:Render()
end
function key(...)
  stdwrap("key", ...)
end
