
achievement = {}

achievement.db = {}
local achievement_params = {}

local achievement_list = {}

local loader_glob = {}
loader_glob.achievement_db = achievement.db
loader_glob.achievement_params = achievement_params
function loader_glob.achievement(param)
  local dat = {text = param[1], points = param[2], icon = param[3]}
  table.insert(achievement_list, dat)
  return {id = #achievement_list, dat = dat}
end

local achievement_persist = {}
persistence.load("achievements", achievement_persist)

runfile("achievements.lua", loader_glob)

local function make_achievement_badge(dat)
  local cheev = CreateFrame("Sprite", overlay)
  local icon = CreateFrame("Sprite", cheev)
  cheev:SetTexture(achievement_params.bgimage)
  icon:SetTexture(dat.icon)
  
  icon:SetPoint(nil, 0.5, cheev, nil, 0.5)
  icon:SetPoint("LEFT", cheev, "LEFT", (cheev:GetHeight() - icon:GetWidth()) / 2)
  local tex = CreateFrame("Text_Multiline", cheev)
  tex:SetPoint("TOPLEFT", icon, "TOPRIGHT", 10, 0)
  tex:SetPoint("RIGHT", cheev, "RIGHT", -10, 0)
  tex:SetPoint("BOTTOM", icon, "BOTTOM")
  tex:SetText(dat.text)

  return cheev, icon, tex
end

function achievement.award(link)
  assert(achievement_list[link.id] == link.dat)
  
  if not achievement_persist[link.dat.icon] then
    local cheev, icon, tex = make_achievement_badge(link.dat)
    
    local fades = 20
    cheev.Tick = coroutine.wrap(function ()
      for i = 1, fades do
        cheev:SetPoint("BOTTOMRIGHT", UIParent, "BOTTOMRIGHT", -5, fades * 2 - i * 2 - 5)
        cheev:SetColor(1, 1, 1, i / fades)
        icon:SetColor(1, 1, 1, i / fades)
        tex:SetColor(1, 1, 1, i / fades)
        coroutine.yield()
      end
      
      coroutine.pause(150)
      
      for i = 1, fades do
        cheev:SetPoint("BOTTOMRIGHT", UIParent, "BOTTOMRIGHT", -5, i * 2 - 5)
        cheev:SetColor(1, 1, 1, 1 - i / fades)
        icon:SetColor(1, 1, 1, 1 - i / fades)
        tex:SetColor(1, 1, 1, 1 - i / fades)
        coroutine.yield()
      end
      
      cheev:Detach()
    end)
    cheev:Tick()
    
    achievement_persist[link.dat.icon] = true
    persistence.save()
  end
end