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

  assert(type(last[lastword]) == "table")
  assert(not next(last[lastword]))
  last[lastword] = item

  return item
end

local function dump_worker(key, value, indent, seen)
  if type(value) == "table" then
    if seen[value] then
      External.print(("  "):rep(indent) .. tostring(key) .. ": (infinite loop)")    
    else
      seen[value] = true
      External.print(("  "):rep(indent) .. tostring(key) .. ":")
      
      local sorted = {}
      for k, v in pairs(value) do
        table.insert(sorted, k)
      end
      table.sort(sorted, function (a, b)
        if type(a) ~= type(b) then return type(a) < type(b) end
        return a < b
      end)
      
      for _, k in ipairs(sorted) do
        dump_worker(k, value[k], indent + 1, seen)
      end
    end
  else
    External.print(("  "):rep(indent) .. tostring(key) .. ": " .. tostring(value))
  end
end
function dump(item)
  dump_worker("dump", item, 0, {})
end
External.dump = dump
