-- Error and debug console.

-- rig up console
local cbacking = Frame.Frame(Frame.Root)
cbacking:SetBackground(0.05, 0.05, 0.05, 0.95)
cbacking:SetStrata(100)
cbacking:SetVisible(false)

cbacking:SetPoint("TOPLEFT", Frame.Root, "TOPLEFT")
cbacking:SetPoint("RIGHT", Frame.Root, "RIGHT")
cbacking:SetPoint("BOTTOM", Frame.Root, nil, 0.75)

local tentry = Frame.Text(cbacking)
tentry:SetPoint("BOTTOMLEFT", cbacking, "BOTTOMLEFT", 2, -2)
tentry:SetPoint("RIGHT", cbacking, "RIGHT", -2, nil)
tentry:SetInteractive("edit")

local lin = Frame.Frame(cbacking)
lin:SetHeight(1)
lin:SetPoint("BOTTOM", tentry, "TOP", nil, -2)
lin:SetPoint("LEFT", Frame.Root, "LEFT")
lin:SetPoint("RIGHT", Frame.Root, "RIGHT")
lin:SetBackground(1, 1, 1)

local dumptext = Frame.Text(cbacking)
dumptext:SetInteractive("select")
dumptext:SetWordwrap(true)
dumptext:SetPoint("BOTTOM", lin, "TOP", nil, -2)
dumptext:SetPoint("LEFT", Frame.Root, "LEFT", 2, nil)
dumptext:SetPoint("RIGHT", Frame.Root, "RIGHT", -2, nil)

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

tentry:EventAttach(Frame.Event.Key.Type, function (f, eh, typ)
  if typ == "\n" then
    --eh:Finalize()
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

External.Event.System.Key.Down:Attach(function (ki)
  if ki == "Escape" and cbacking:GetVisible() then
    cbacking:SetVisible(false)
    tentry:SetFocus(false)
    tentry:SetText("")
  end
end)
External.Event.System.Key.Type:Attach(function (ki)
  if ki == "/" and not cbacking:GetVisible() then
    cbacking:SetVisible(true)
    tentry:SetFocus(true)
    tentry:SetText("")
  end
end)