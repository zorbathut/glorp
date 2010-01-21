
menu = {}

menu.ok_button = CreateFrame("Button")
menu.ok_text = CreateFrame("Text", menu.ok_button)
menu.ok_text:SetColor(1, 1, 1)
menu.ok_text:SetSize(40)
menu.ok_text:SetText("New Game")

menu.exit_button = CreateFrame("Button")
menu.exit_text = CreateFrame("Text", menu.exit_button)
menu.exit_text:SetColor(1, 1, 1)
menu.exit_text:SetSize(40)
menu.exit_text:SetText("Exit")

menu.achievement_button = CreateFrame("Button")
menu.achievement_text = CreateFrame("Text", menu.achievement_button)
menu.achievement_text:SetColor(1, 1, 1)
menu.achievement_text:SetSize(40)
menu.achievement_text:SetText("Achievements")

--[[
local ptr = CreateFrame("Text")
ptr:SetSize(40)
ptr:SetColor(1, 1, 1)
ptr:SetText(">")

local anchor = menu.ok_text

local function spt()
  ptr:SetPoint(1, 0.5, anchor, 0, 0.5, -10, 0)
end
spt()]]


function menu.ok_button:Click()
  GlorpController("start_game")
end
function menu.exit_button:Click()
  GlorpController("exit")
end
function menu.achievement_button:Click()
  print("cheevz")
  --GlorpController("exit")
end

function key(button, ascii, event)
  if button == "escape" and event == "press" then
    GlorpController("exit")
  end
  
  --[[
  if (button == "enter" or button == "z" or button == "x" or button == "c") and event == "press" then
    if anchor == menu.ok_text then
      GlorpController("start_game")
    else
      GlorpController("exit")
    end
  end
  
  if (button == "arrow_up" or button == "arrow_down") and event == "press" then
    if anchor == menu.ok_text then
      anchor = menu.exit_text
    else
      anchor = menu.ok_text
    end
    spt()
  end]]
end

runfile("menu.lua", _G)
