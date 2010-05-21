
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

runfile_optional("achievements.lua", loader_glob)

local height = 100
local width_small = 300
local width_full = 450

local function make_achievement_badge(dat, full, parent)
  local cheev = CreateFrame("Frame", parent or overlay)
  local icon = CreateFrame("Texture", cheev)
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
  
  if full then
    tex:SetText("\1B\1" .. dat.text)
    print("testing", dat.icon, achievement_list[dat.icon])
    if achievement_persist[dat.icon] then
      print("bgcoloring")
      cheev:SetBackgroundColor(0, 0.3, 0, 1)
    end
    
    local fult = CreateFrame("Text_Multiline", cheev)
    fult:SetPoint("TOPLEFT", icon, 1, 0.5, 10, 0)
    fult:SetPoint("RIGHT", cheev, "RIGHT", -10, 0)
    fult:SetPoint("BOTTOM", icon, "BOTTOM")
    fult:SetText("\1S0.8\1" .. dat.descr)
  else
    tex:SetText(dat.text)
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

-- this is terrible but I need to get this done
function achievement.display()
  print("entering")
  local dun = false
  
  local achbg = CreateFrame("Button", overlay)
  function achbg:Click()
    dun = true
  end
  achbg:SetBackgroundColor(0, 0, 0, 0.5)
  achbg:SetAllPoints(UIParent)
  achbg:SetLayer(10)
  
  local vpos = 1
  for _, v in pairs(achievement_list) do
    print("makin' achieve")
    
    local x, y = math.mod(vpos - 1, 2) + 1, math.floor((vpos - 1) / 2) + 1
    
    local baj = make_achievement_badge(v, true, achbg)
    baj:SetPoint("CENTER", UIParent, (x - 1) * 0.5 + 0.25, (y - 1) / 5 + 0.1)
    
    vpos = vpos + 1
  end
  
  while not dun do coroutine.yield() end
  
  achbg:Detach()
end
