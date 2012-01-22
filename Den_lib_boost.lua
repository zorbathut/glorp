local params = ...

local headers = params.headers
local libs = params.libs
local platform = params.platform
local builddir = params.builddir

local boost_dir = "glorp/libs/boost_1_45_0"
  
ursa.token.rule{"boost files", nil, ("cd %s && find . -type f | sed s*\\\\./**"):format(boost_dir)}  -- we are assuming this isn't changing
local boost_files = {}
for file in ursa.token{"boost files"}:gmatch("([^\n]+)") do
  table.insert(boost_files, ursa.rule{builddir .. "lib_release/include/" .. file, boost_dir .. "/" .. file, ursa.util.copy{}})
end

headers.boost = boost_files
libs.boost = {}
