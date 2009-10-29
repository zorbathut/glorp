
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
local inminimenu

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

local minimenu_frames = CreateFrame("Frame")
minimenu_frames:SetAllPoints()
minimenu_frames:SetLayer(10000000) -- this is really only needed so that clickery occurs properly

local minimenu_backdrop = CreateFrame("Frame", minimenu_frames)
minimenu_backdrop:SetAllPoints()
minimenu_backdrop:SetLayer(-1)
minimenu_backdrop:SetBackgroundColor(0, 0, 0, 0.9)

minimenu_paused = CreateFrame("Text", minimenu_frames)
minimenu_paused:SetText("Paused")
minimenu_paused:SetSize(60)
minimenu_paused:SetColor(1, 0.5, 0.5)
minimenu_paused:SetPoint("CENTER", UIParent, "CENTER", 0, -200)

minimenu_resume_button = CreateFrame("Button", minimenu_frames)
minimenu_resume = CreateFrame("Text", minimenu_resume_button)
minimenu_resume:SetText("Resume")
minimenu_resume:SetSize(30)
minimenu_resume:SetColor(1, 1, 1)
minimenu_resume:SetPoint("CENTER", UIParent, "CENTER", 0, -50)
local function imm_end()
  minimenu_frames:Hide()
  inminimenu = false
end
function minimenu_resume_button:Click()
  print("clique")
  imm_end()
end

minimenu_return_button = CreateFrame("Button", minimenu_frames)
minimenu_return = CreateFrame("Text", minimenu_return_button)
minimenu_return:SetText("Return to main menu")
minimenu_return:SetSize(30)
minimenu_return:SetColor(1, 1, 1)
minimenu_return:SetPoint("CENTER", UIParent, "CENTER", 0, 50)
function minimenu_return_button:Click()
  print("clique")
  Handle("exit_game")
  inminimenu = false
end

minimenu_left = CreateFrame("Text", minimenu_frames)
minimenu_left:SetText(">")
minimenu_left:SetSize(30)
minimenu_left:SetColor(1, 1, 1)
minimenu_right = CreateFrame("Text", minimenu_frames)
minimenu_right:SetText("<")
minimenu_right:SetSize(30)
minimenu_right:SetColor(1, 1, 1)


minimenu_frames:Hide()

local minimenu_pos = 1

local function imm_resync()
  local junct = (minimenu_pos == 1) and minimenu_resume or minimenu_return
  
  minimenu_left:SetPoint(1, 0.5, junct, 0, 0.5, -20, 0)
  minimenu_right:SetPoint(0, 0.5, junct, 1, 0.5, 20, 0)
end

local function imm_start()
  minimenu_frames:Show()
  minimenu_pos = 1
  imm_resync()
  inminimenu = true
end
local function imm_render()
  minimenu_frames:Render()
end

local function imm_key(button, ascii, event)
  print("immkey", button, ascii, event)
  if (button == "arrow_down" or button == "arrow_up") and event == "press" then
    minimenu_pos = (minimenu_pos == 1) and 2 or 1
    imm_resync()
  end
  
  if (button == "enter" or button == "space") and event == "press" then
    local junct = (minimenu_pos == 1) and minimenu_resume_button or minimenu_return_button
    junct:Click()
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
  if not inminimenu then
    stdwrap("tick_loop", ...)
  end
end
function loop(...)
  if not inminimenu then
    stdwrap("loop", ...)
  end
end
function render(...)
  stdwrap("render", ...)
  local context = runninggame or mainmenu
  context.UIParent:Render()
  
  if inminimenu then
    imm_render()
  end
end
function key(button, ascii, event)
  print(button, ascii, event)
  if runninggame and button == "escape" and event == "press" then
    print("preep")
    if inminimenu then
      print("imme")
      imm_end()
    else
      print("imms")
      imm_start()
    end
  elseif inminimenu then
    print("eem")
    imm_key(button, ascii, event)
  else
    print("wut")
    stdwrap("key", button, ascii, event)
  end
end
