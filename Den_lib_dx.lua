local params = ...

local headers = params.headers
local libs = params.libs
local platform = params.platform
local builddir = params.builddir
local addHeadersFor = params.addHeadersFor

local dx_dir_inc = "/cygdrive/c/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Include"
local dx_dir_lib = "/cygdrive/c/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Lib/x86"

ursa.token.rule{"dx inc files", nil, ("cd \"%s\" && find . -type f | sed s*\\\\./**"):format(dx_dir_inc)}  -- we are assuming this isn't changing
ursa.token.rule{"dx lib files", nil, ("cd \"%s\" && find . -type f | sed s*\\\\./**"):format(dx_dir_lib)}  -- we are assuming this isn't changing

libs.dx = {}
headers.dx = {}

-- now we copy shit to the right place

for file in ursa.token{"dx inc files"}:gmatch("([^\n]+)") do
  table.insert(headers.dx, ursa.rule{builddir .. "lib_release/include/" .. file, dx_dir_inc .. "/" .. file, ursa.util.copy{}})
end
for file in ursa.token{"dx lib files"}:gmatch("([^\n]+)") do
  table.insert(headers.dx, ursa.rule{builddir .. "lib_release/lib/" .. file, dx_dir_lib .. "/" .. file, ursa.util.copy{}})
end

