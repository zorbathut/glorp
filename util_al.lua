
-- tables in tables exist to fix a luabind crash

local sources_active = {}
local sources = setmetatable({}, {__mode = "k"})

alutil = {}
function alutil.Source()
  local src = AlSourceID()
  sources[src] = true
  
  return src
end

function alutil.Tick()
  for k in pairs(sources) do
    if al.GetSource(k:get(), "SOURCE_STATE") ~= "PLAYING" then
      sources_active[k] = nil
    else
      sources_active[k] = true
    end
  end
end
