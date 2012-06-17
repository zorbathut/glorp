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

InsertItem(External, "Command.Environment.Create", function (root, label)
  assert(root)
  assert(label)
  
  local nenv = CopyDeep(basic)
  nenv._G = nenv
  
  nenv.Command = CopyDeep(root.Command)
  nenv.Inspect = CopyDeep(root.Inspect)
  nenv.Utility = CopyDeep(root.Utility)
  
  nenv.Frames.Root = nenv.Frames.Frame(basic.Frames.Root)
  nenv.Frames.Root:SetAllPoints(basic.Frames.Root)
  
  -- TODO: event

  return nenv
end)
