
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

function menu.ok_button:Click()
  GlorpController("start_game")
end
function menu.exit_button:Click()
  GlorpController("exit")
end

function key(button, ascii, event)
  if button == "escape" and event == "press" then
    GlorpController("exit")
  end
  
  if button == "enter" and event == "press" then
    GlorpController("start_game")
  end
end

loadfile("menu.lua")()
