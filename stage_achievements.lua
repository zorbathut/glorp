
achievement = {}

achievement.db = {}
local achievement_params = {}

local achievement_list = {}

local loader_glob = {}
loader_glob.achievement_db = achievement.db
loader_glob.achievement_params = achievement_params
function loader_glob.achievement(param)
  local dat = {text = param[1], descr = param[2], icon = param[3]}
  table.insert(achievement_list, dat)
  return {id = #achievement_list, dat = dat}
end

local achievement_persist = persistence.load("achievements")

runfile("achievements.lua", loader_glob)

local height = 100
local width_small = 300
local width_full = 900

local function make_achievement_badge(dat, full)
  local cheev = CreateFrame("Frame", overlay)
  local icon = CreateFrame("Sprite", cheev)
  cheev:SetBackgroundColor(0, 0, 0, 1)
  cheev:SetWidth(full and width_full or width_small)
  cheev:SetHeight(height)
  
  icon:SetTexture(dat.icon)
  
  icon:SetPoint(nil, 0.5, cheev, nil, 0.5)
  icon:SetPoint("LEFT", cheev, "LEFT", (cheev:GetHeight() - icon:GetWidth()) / 2)
  local tex = CreateFrame("Text_Multiline", cheev)
  tex:SetPoint("TOPLEFT", icon, "TOPRIGHT", 10, 0)
  tex:SetPoint("RIGHT", cheev, "RIGHT", -10, 0)
  tex:SetPoint("BOTTOM", icon, "BOTTOM")
  tex:SetText(dat.text)
  
  if full then
    local fult = CreateFrame("Text_Multiline", cheev)
    fult:SetPoint("TOPLEFT", icon, 1, 0.5, 10, 0)
    fult:SetPoint("RIGHT", cheev, "RIGHT", -10, 0)
    fult:SetPoint("BOTTOM", icon, "BOTTOM")
    fult:SetText(dat.descr)
  end

  return cheev, icon, tex
end

local slots_used = {}

local soundable = true

function achievement.award(link)
  assert(achievement_list[link.id] == link.dat)
  
  if not achievement_persist[link.dat.icon] then
    local cheev, icon, tex = make_achievement_badge(link.dat)
    
    local slot = 1
    while slots_used[slot] do slot = slot + 1 end
    slots_used[slot] = true
    
    if soundable then
      PlaySound("achieve")
      soundable = false
    end
    
    local fades = 20
    cheev.Tick = coroutine.wrap(function ()
      for i = 1, fades do
        cheev:SetPoint("BOTTOMRIGHT", UIParent, "BOTTOMRIGHT", -5, fades * 2 - i * 2 - 5 - (slot - 1) * (height + 5))
        cheev:SetBackgroundColor(0, 0, 0, i / fades)
        icon:SetColor(1, 1, 1, i / fades)
        tex:SetColor(1, 1, 1, i / fades)
        coroutine.yield()
      end
      
      coroutine.pause(150)
      soundable = true
      
      for i = 1, fades do
        cheev:SetPoint("BOTTOMRIGHT", UIParent, "BOTTOMRIGHT", -5, i * 2 - 5 - (slot - 1) * (height + 5))
        cheev:SetBackgroundColor(0, 0, 0, 1 - i / fades)
        icon:SetColor(1, 1, 1, 1 - i / fades)
        tex:SetColor(1, 1, 1, 1 - i / fades)
        coroutine.yield()
      end
      
      slots_used[slot] = nil
      cheev:Detach()
    end)
    cheev:Tick()
    
    achievement_persist[link.dat.icon] = true
    persistence.save()
  end
end