
local platform, _, mode = ...

function runfile_worker(file, global, optional, ...)
  assert(global)
  
  local dat, rv = loadfile(file)
  
  if rv and rv:find("No such file or directory") then
    dat, rv = loadfile("data/" .. file)
  end
  
  if rv and rv:find("No such file or directory") then
    dat, rv = loadfile("glorp/" .. file)
  end
  
  if rv then
    if optional and rv:find("No such file or directory") then return end
    
    assert(false, rv)
  end
  
  if global then
    setfenv(dat, global)
  end
  
  dat(mode, platform, ...)
end
function runfile(file, global, ...)
  runfile_worker(file, global, false, ...)
end
function runfile_optional(file, global, ...)
  runfile_worker(file, global, true, ...)
end

runfile("util.lua", _G)
runfile("ui.lua", _G)

runfile("stage_persistence.lua", _G)
runfile("stage_achievements.lua", _G)

if not jit and mode == "debug" then
  runfile("pepperfish.lua", _G)
  pepperfish_profiler = newProfiler()
  pepperfish_profiler:start()
end

local mainmenu
local runninggame
local inminimenu

local function destroy_game()
  if not runninggame then return end
  if runninggame.shutdown then runninggame.shutdown() end
  runninggame.UIRoot:Detach() -- yunk
  runninggame = nil
end
local function reset_menu()
  mainmenu, mainmenu_ui = runuifile("menu_core.lua")
end
local function Handle(param, ...)
  if not param then return end
  
  if param == "start_game" then
    destroy_game()
    if mode == "editor" then
      runninggame = runuifile("editor.lua", ...)
    else
      runninggame = runuifile("main.lua", ...)
    end
    mainmenu.UIRoot:Hide()
  elseif param == "exit_game" then
    destroy_game()
    reset_menu()
    mainmenu.UIRoot:Show()
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



overlay = CreateFrame("Frame")
overlay:SetLayer(1000000)

function runuifile(file, ...)
  local env = {}
  for k, v in pairs(_G) do
    env[k] = v
  end
  
  env._G = env
  
  local uip = CreateFrame("Frame")
  uip:SetAllPoints()
  env.UIRoot = uip
  -- hackery hackhack
  if GetScreenX() == 320 and GetScreenY() == 480 then
    uip:SetCoordinateScale(512, 384, 1024, 0)
    env.UIParent = CreateFrame("Frame", uip)
    env.UIParent:SetPoint("TOPLEFT", UIParent, "TOPLEFT")
    env.UIParent:SetPoint("BOTTOMRIGHT", UIParent, "TOPLEFT", 1024, 768)  -- we need for uiparent to have the right coordinates
  else
    env.UIParent = uip
  end
  
  env.CreateFrame = function (type, parent) return CreateFrame(type, parent or env.UIParent) end
  env.GlorpController = Handle
  env.loadfile = function (...)
    local dat, rv = loadfile(...)
    if dat then setfenv(dat, env) end
    return dat, rv
  end
  
  env.tick_loop = nil
  env.loop = nil
  env.render = nil
  env.key = nil
  env.failover = nil
  
  runfile(file, env, ...)
  
  return env, uip
end
reset_menu()

function stdwrap(token, ...)
  local context = runninggame or mainmenu
  
  if context[token] then
    return context[token](...)
  end
end

mainmenu_ui:Hide()

local bgbg = CreateFrame("Frame")
bgbg:SetBackgroundColor(mainmenu.bg_r or 0, mainmenu.bg_g or 0, mainmenu.bg_b or 0)
bgbg:SetAllPoints()
local lojo = CreateFrame("Sprite", bgbg)
lojo:SetTexture(Texture("mandible_games", "../glorp/resources/mandible_games"))
lojo:SetPoint("CENTER", bgbg, "CENTER")
lojo:SetColor(1, 1, 1, 0)
local lojo_l = CreateFrame("Frame", bgbg)
lojo_l:SetPoint("TOPLEFT", bgbg, "TOPLEFT")
lojo_l:SetPoint("BOTTOM", bgbg, "BOTTOM")
lojo_l:SetPoint("RIGHT", lojo, "LEFT")
local lojo_r = CreateFrame("Frame", bgbg)
lojo_r:SetPoint("TOPRIGHT", bgbg, "TOPRIGHT")
lojo_r:SetPoint("BOTTOM", bgbg, "BOTTOM")
lojo_r:SetPoint("LEFT", lojo, "RIGHT")
local lojo_u = CreateFrame("Frame", bgbg)
lojo_u:SetPoint("TOP", bgbg, "TOP")
lojo_u:SetPoint("BOTTOM", lojo, "TOP")
lojo_u:SetPoint("LEFT", lojo, "LEFT")
lojo_u:SetPoint("RIGHT", lojo, "RIGHT")
local lojo_d = CreateFrame("Frame", bgbg)
lojo_d:SetPoint("TOP", lojo, "BOTTOM")
lojo_d:SetPoint("BOTTOM", bgbg, "BOTTOM")
lojo_d:SetPoint("LEFT", lojo, "LEFT")
lojo_d:SetPoint("RIGHT", lojo, "RIGHT")

local function fadedafucker(st, nd, len)
  for i = 1, len do
    local diff = i / len
    local amt = nd * diff + st * (1 - diff)
    lojo:SetColor(1, 1, 1, amt)
    lojo_l:SetBackgroundColor(1, 1, 1, amt)
    lojo_r:SetBackgroundColor(1, 1, 1, amt)
    lojo_u:SetBackgroundColor(1, 1, 1, amt)
    lojo_d:SetBackgroundColor(1, 1, 1, amt)
    coroutine.yield()
  end
end

local ssmessage = nil

local somethingpressed
local wedothisfirst
wedothisfirst = coroutine.wrap(function()
  coroutine.pause(10)
  fadedafucker(0, 1, 10)
  for i = 1, 120 do
    if somethingpressed then break end
    coroutine.yield()
  end
  fadedafucker(1, 0, 20)
  coroutine.pause(10)
  
  bgbg:Hide()
  wedothisfirst = nil
  
  mainmenu_ui:Show()
end)

function tick_loop(...)
  UIParent:Update()
  
  if wedothisfirst then
    wedothisfirst()
  else
    if not inminimenu then
      stdwrap("tick_loop", ...)
    end
  end
end
function loop(...)
  if wedothisfirst then return end
  
  if not inminimenu then
    stdwrap("loop", ...)
  end
end
function render(...)
  gl.Disable("CULL_FACE") -- ffffffff
  
  if wedothisfirst then bgbg:Render() return end
  
  stdwrap("render", ...)
  local context = runninggame or mainmenu
  context.UIRoot:Render()
  
  if inminimenu then
    imm_render()
  end
  
  overlay:Render()
end
function key(button, ascii, event)
  if button == "printscreen" and event == "press" then
    print("printscr")
    
    local fname = string.format("%s_%d.png", GetMidName(), os.time())
    
    local path = GetDesktopDirectory() .. "/" .. fname
    assert(ScreenshotTo(path))
    
    if ssmessage then ssmessage:Detach() ssmessage = nil end
    
    ssmessage = CreateFrame("Text_Multiline", overlay)
    ssmessage:SetAllPoints()
    ssmessage.tixleft = 240
    ssmessage.tixfade = 60
    ssmessage:SetLayer(100000000)
    local txt = "Screenshot saved to " .. GetDesktopDirectory() .. "\\" .. fname
    function ssmessage:Tick()
      print("tixtix", self.tixleft)
      self.tixleft = self.tixleft - 1
      if self.tixleft == 0 then self:Detach() ssmessage = nil end
      if self.tixleft > self.tixfade then
        self:SetText("\1C000000ff\1" .. txt)
      else
        self:SetText(("\1C000000%02x\1"):format(self.tixleft / self.tixfade * 255) .. txt)
      end
    end
    ssmessage:Tick()
  end
  
  if wedothisfirst then
    if event == "press" then
      somethingpressed = true
    end
    return
  end
  
  if runninggame and button == "escape" and event == "press" then
    if inminimenu then
      imm_end()
    else
      imm_start()
    end
  elseif inminimenu then
    imm_key(button, ascii, event)
  else
    stdwrap("key", button, ascii, event)
  end
end
function failover()
  stdwrap("failover")
end

if mode == "debug" then
  Perfbar_Set(true)
  wedothisfirst = nil
  Handle("start_game")
end
if mode == "editor" then
  wedothisfirst = nil
  Handle("start_game")
end
