

local version = gl.GetString("VERSION"):match("^(%d+%.%d+).*")
if tonumber(version) < gl_version_expected then
  local tex = CreateFrame("Text_Multiline")
  
  tex:SetText("This game requires some nontrivial graphics card capabilities. Unfortunately, your graphics card doesn't seem to be capable of them. You might be able to solve this by updating your drivers, but it's also possible that you simply don't have a graphics card that is able to do them.\n\nThis means you can't play the game. Sorry! I'd really love it if you could, but with two days to write an entire game, certain things (like compatibility) must be sacrificed. Come back next month for another game!\n\n(If you're curious, this game requires OpenGL 2.0.)")
  tex:SetPoint("CENTER", UIParent, "CENTER")
  tex:SetWidth(600)
  tex:SetColor(1, 1, 1)
  tex:ForceHeight()

  menu.ok_text:Hide()
  menu.exit_text:Hide()
  
  disable_anchor()
  
  return
end



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
menu.achievement_button:Hide() -- we don't want this anymore


local ptr = CreateFrame("Text")
ptr:SetSize(40)
ptr:SetColor(1, 1, 1)
ptr:SetText(">")

local anchor = menu.ok_text

local function spt()
  ptr:SetPoint(1, 0.5, anchor, 0, 0.5, -10, 0)
end
spt()


function menu.ok_button:Click()
  GlorpController("start_game")
end
function menu.exit_button:Click()
  GlorpController("exit")
end

local allow_anchor = true
function disable_anchor()
  allow_anchor = false
  
  ptr:Hide()
end


function key(button, ascii, event)
  if button == "escape" and event == "press" then
    GlorpController("exit")
  end
  
  if allow_anchor then
    if (button == "enter" or button == "z" or button == "x" or button == "c" or button == "space") and event == "press" then
      if anchor == menu.ok_text then
        menu.ok_button:Click()
      else
        menu.exit_button:Click()
      end
    end
    
    if (button == "arrow_up" or button == "arrow_down") and event == "press" then
      if anchor == menu.ok_text then
        anchor = menu.exit_text
      else
        anchor = menu.ok_text
      end
      spt()
    end
  end
end

runfile("menu.lua", _G)
