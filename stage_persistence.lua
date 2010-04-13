
local mode = ...

MakeConfigDirectory()
local pfile = GetConfigDirectory() .. "saved.lua"

persistence = {}

local loaded_from = nil
if mode ~= "debug" then
  local r, v = loadfile(pfile)
  if r then
    loaded_from = r()
  end
end

if not loaded_from then
  loaded_from = {}
end

function persistence.load(token)
  if not loaded_from[token] then
    loaded_from[token] = {}
  end
  
  assert(loaded_from[token])
  
  return loaded_from[token]
end
function persistence.save()
  local fil = io.open(pfile, "wb")
  assert(fil)
  fil:write("return " .. persistence.stringize(loaded_from))
  fil:close()
end



local function Merge_Add(self, data, stoprepeat)  -- NOTE: if you're getting errors about adding tables, you probably did Merger:Add instead of Merger.Add
  if stoprepeat and #self > 0 and string.sub(self[#self], -#data) == data then return end
  table.insert(self, data)
  for i = #self - 1, 1, -1 do
    if string.len(self[i]) > string.len(self[i + 1]) then break end
    self[i] = self[i] .. table.remove(self, i + 1)
  end
end
local function Merge_Finish(self, data)
  for i = #self - 1, 1, -1 do
    self[i] = self[i] .. table.remove(self)
  end
  return self[1] or ""
end

local persist
persist =
{
  store = function (f, item)
    if f then
      persist.write(f, item, 0);
      f:write("\n");
    else
      error(e);
    end;
  end;
  
  stringize = function (item)
    local st = {}
    persist.store({write = function(_, txt) Merge_Add(st, txt) end}, item)
    return Merge_Finish(st)
  end,
  
  load = function (path)
    local f, e = loadfile(path);
    if f then
      return f();
    else
      return nil, e;
      --error(e);
    end;
  end;
  
  write = function (f, item, level)
    local t = type(item);
    persist.writers[t](f, item, level);
  end;
  
  writeIndent = function (f, level)
    for i = 1, level do
      f:write("\t");
    end;
  end;
  
  writers = {
    ["nil"] = function (f, item, level)
        f:write("nil");
      end;
    ["number"] = function (f, item, level)
        f:write(tostring(item));
      end;
    ["string"] = function (f, item, level)
        f:write(string.format("%q", item));
      end;
    ["boolean"] = function (f, item, level)
        if item then
          f:write("true");
        else
          f:write("false");
        end
      end;
    ["table"] = function (f, item, level)
        f:write("{\n");
        
        local first = true
        local hit = {}
        
        persist.writeIndent(f, level+1);
        
        for k, v in ipairs(item) do
          hit[k] = true
          if not first then
            f:write(", ")
          end
          
          persist.write(f, v, level+1)
          first = false
        end
        
        local order = {}
        for k, v in pairs(item) do
          if not hit[k] then table.insert(order, k) end
        end
        
        table.sort(order, function (a, b)
          if type(a) == type(b) then return a < b end
          return type(a) < type(b)
        end)
        
        for _, v in pairs(order) do
          if not first then f:write(",\n") end
          first = false
          persist.writeIndent(f, level+1);
          
          if type(v) == "string" and v:match("[a-z]+") then
            f:write(v)
          else
            f:write("[");
            persist.write(f, v, level+1);
            f:write("]");
          end
          f:write(" = ");
          
          persist.write(f, item[v], level+1);
        end
        f:write("\n")
        persist.writeIndent(f, level);
        
        f:write("}");
      end;
    ["function"] = function (f, item, level)
        -- Does only work for "normal" functions, not those
        -- with upvalues or c functions
        local dInfo = debug.getinfo(item, "uS");
        if dInfo.nups > 0 then
          f:write("nil -- functions with upvalue not supported\n");
        elseif dInfo.what ~= "Lua" then
          f:write("nil -- function is not a lua function\n");
        else
          local r, s = pcall(string.dump,item);
          if r then
            f:write(string.format("loadstring(%q)", s));
          else
            f:write("nil -- function could not be dumped\n");
          end
        end
      end;
    ["thread"] = function (f, item, level)
        f:write("nil --thread\n");
      end;
    ["userdata"] = function (f, item, level)
        f:write("nil --userdata\n");
      end;
  }
}

persistence.stringize = persist.stringize