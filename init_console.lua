
-- rig up console
local cbacking = Frames.Frame(Frames.Root)
cbacking:SetBackground(0.05, 0.05, 0.05, 0.95)
cbacking:SetStrata(100)
cbacking:SetVisible(false)

cbacking:SetPoint("TOPLEFT", Frames.Root, "TOPLEFT")
cbacking:SetPoint("RIGHT", Frames.Root, "RIGHT")
cbacking:SetPoint("BOTTOM", Frames.Root, nil, 0.75)

local tentry = Frames.Text(cbacking)
tentry:SetPoint("BOTTOMLEFT", cbacking, "BOTTOMLEFT", 2, -2)
tentry:SetPoint("RIGHT", cbacking, "RIGHT", -2, nil)
tentry:SetInteractive("edit")

local lin = Frames.Frame(cbacking)
lin:SetHeight(1)
lin:SetPoint("BOTTOM", tentry, "TOP", nil, -2)
lin:SetPoint("LEFT", Frames.Root, "LEFT")
lin:SetPoint("RIGHT", Frames.Root, "RIGHT")
lin:SetBackground(1, 1, 1)

local dumptext = Frames.Text(cbacking)
dumptext:SetInteractive("select")
dumptext:SetWordwrap(true)
dumptext:SetPoint("BOTTOM", lin, "TOP", nil, -2)
dumptext:SetPoint("LEFT", Frames.Root, "LEFT", 2, nil)
dumptext:SetPoint("RIGHT", Frames.Root, "RIGHT", -2, nil)

local op = print
local cdump = {}
local function insertline(lin)
  table.insert(cdump, lin)
  if #cdump > 100 then
    table.remove(cdump, 1)
  end
  
  dumptext:SetText(table.concat(cdump, "\n"))
end
local function inserttext(lin)
  for k in lin:gmatch("([^\n]+)") do
    insertline(lin)
  end
end

-- we're going to straight-up permanently hook print
local switch = false
local oldprint = print
local function hookprint(...)
  print(...)
  
  if switch then
    -- send to inserttext
    local res = ""
    for i = 1, select("#", ...) do
      if i ~= 1 then res = res .. '\t' end
      res = res .. tostring(select(i, ...))
    end
    
    inserttext(res)
  end
end
External.print = hookprint

tentry:EventKeyTypeAttach(function (f, eh, typ)
  if typ == "\n" then
    eh:Finalize()
    local command = tentry:GetText()
    tentry:SetText("")
    
    local str, err = loadstring(command)
    if not str then
      inserttext(err)
      return
    end
    
    switch = true
    local suc, res = xpcall(str, function (err) return {err = err, trace = debug.traceback()} end)
    if not suc then
      hookprint(res.err .. "\n" .. res.trace)
    end
    switch = false
  end
end, -1)

table.insert(External.Event.System.Key.Down, function (ki)
  if ki == "Escape" and cbacking:GetVisible() then
    cbacking:SetVisible(false)
    tentry:SetFocus(false)
    tentry:SetText("")
  end
end)
table.insert(External.Event.System.Key.Type, function (ki)
  if ki == "/" and not cbacking:GetVisible() then
    cbacking:SetVisible(true)
    tentry:SetFocus(true)
    tentry:SetText("")
  end
end)