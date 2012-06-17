-- Various utilities that are needed for initialization.

function InsertItem(root, path, item)
  local last = root
  local lastword
  local current = root
  for word in path:gmatch("([^.]+)") do
    current[word] = current[word] or {}
    assert(type(current[word]) == "table")
    last, current, lastword = current, current[word], word
  end

  last[lastword] = item

  return item
end

local function dump_worker(key, value, indent, seen)
  if type(value) == "table" then
    if seen[value] then
      print(("  "):rep(indent) .. tostring(key) .. ": (infinite loop)")    
    else
      seen[value] = true
      print(("  "):rep(indent) .. tostring(key) .. ":")
      for k, v in pairs(value) do
        dump_worker(k, v, indent + 1, seen)
      end
    end
  else
    print(("  "):rep(indent) .. tostring(key) .. ": " .. tostring(value))
  end
end
function dump(item)
  dump_worker("dump", item, 0, {})
end
External.dump = dump

