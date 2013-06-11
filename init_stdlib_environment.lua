-- Environment creation and destruction framework. Must be called after the global state is set up.

-- First, we make a deep copy of the existing environment
function CopyDeep(src)
  if type(src) == "table" then
    local rv = {}
    for k, v in pairs(src) do
      rv[CopyDeep(k)] = CopyDeep(v)
    end
    return rv
  else
    return src  -- lulz "copy"
  end
end

local basic = {}
for k, v in pairs(External) do
  -- we kill _G because of infinite loops
  -- we kill package because package.loaded causes infinite loops, and honestly, I don't use package for anything
  if k ~= "_G" and k ~= "package" then
    basic[CopyDeep(k)] = CopyDeep(v)
  end
end
basic.Event = nil
basic.Command = nil
basic.Inspect = nil
basic.Utility = nil
basic.Frames.Root = nil

local function RecursiveEventRecreate(target, source, context, eventlist)
  for k, v in pairs(source) do
    assert(type(v) == "table")
    if type(v) ~= "table" then return end
    
    if v.CreateContextHandle then
      target[k] = v:CreateContextHandle(context)
      local blob = target[k]
      table.insert(eventlist, function () blob:ClearContext(context) end)
    else
      target[k] = {}
      RecursiveEventRecreate(target[k], source[k], context, eventlist)
    end
  end
end

local contextlist = {}
local contextshutdown = setmetatable({}, {__mode = 'k'})

InsertItem(External, "Command.Environment.Create", function (root, label, ...)
  assert(root)
  assert(label)
  
  local contextmeta = {}
  contextmeta.teardown = {}
  
  local nenv = CopyDeep(basic)
  nenv._G = nenv
  
  nenv.Command = CopyDeep(root.Command)
  nenv.Inspect = CopyDeep(root.Inspect)
  nenv.Utility = CopyDeep(root.Utility)
  
  nenv.Frames.Root = nenv.Frames.Frame(root.Frames.Root)
  nenv.Frames.Root:SetPoint("TOPLEFT", root.Frames.Root, "TOPLEFT")
  nenv.Frames.Root:SetPoint("BOTTOMRIGHT", root.Frames.Root, "BOTTOMRIGHT")
  
  nenv.Event = {}
  
  RecursiveEventRecreate(nenv.Event, root.Event, nenv, contextmeta.teardown)
  
  contextlist[nenv] = contextmeta
  
  for k = 1, select("#", ...) do
    local v = select(k, ...)
    if type(v) == "function" then
      v(nenv)
    elseif type(v) == "string" then
      setfenv(assert(loadfile(v)), nenv)(param)
    else
      assert(false, "Unknown type fed into Command.Environment.Create")
    end
  end
  
  return nenv
end)

InsertItem(External, "Command.Environment.Destroy", function (target)
  local meta = contextlist[target]
  assert(meta)
  if not meta then return end
  
  for _, v in ipairs(meta.teardown) do
    v()
  end
  
  target.Frames.Root:Obliterate()
  
  contextlist[target] = nil
  contextshutdown[target] = External.Inspect.System.Time.Real()
end)

External.Event.System.Update.Begin:Attach(function ()
  for k, v in pairs(contextshutdown) do
    assert(v > External.Inspect.System.Time.Real() - 15)
  end
end)
