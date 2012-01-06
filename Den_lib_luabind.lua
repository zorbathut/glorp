local params = ...

local headers = params.headers
local libs = params.libs
local platform = params.platform
local builddir = params.builddir
local addHeadersFor = params.addHeadersFor

addHeadersFor("lua")

headers.luabind = {}
local path = "glorp/libs/luabind-0.9"

local files = {}

ursa.token.rule{"luabind_files", nil, function () return ursa.system{("cd %s && find . -type f | grep -v test | grep -v examples | sed s*\\\\./**"):format(path)} end}

for item in ursa.token{"luabind_files"}:gmatch("([^\n]+)") do
  if item == "luabind/adopt_policy.hpp" then
    local sourcefile = path .. "/" .. item
    local patchfile = "glorp/libs/luabind_adopt_container.patch"
    table.insert(files, ursa.rule{builddir .. "lib_build/luabind/" .. item, {sourcefile, patchfile}, ursa.util.system_template{("cp %s $TARGET && cd %s && patch -p 1 -i ../../../../%s"):format(sourcefile, builddir .. "lib_build/luabind", patchfile)}})
  else
    table.insert(files, ursa.rule{builddir .. "lib_build/luabind/" .. item, path .. "/" .. item, ursa.util.copy{}})
  end    
  
  if item:match("%.hpp$") then
    table.insert(headers.luabind, ursa.rule{builddir .. "lib_release/include/" .. item, builddir .. "lib_build/luabind/" .. item, ursa.util.copy{}})
  end
end

local objs = {}
for item in ursa.token{"luabind_files"}:gmatch("([^\n]+)") do
  if item:match("%.cpp$") then
    table.insert(objs, ursa.rule{builddir .. "lib_build/luabind/" .. item:gsub(".cpp", ".o"), {headers.lua, headers.luabind, builddir .. "lib_build/luabind/" .. item}, ursa.util.system_template{("nice #CC #optflags -o $TARGET -c %s -I#BUILDDIR/lib_release/include -Iglorp/libs/boost_1_45_0 #CXXFLAGS -g"):format(builddir .. "lib_build/luabind/" .. item)}})
  end
end

local luabind = builddir .. "lib_release/lib/libluabind.a"
libs.luabind = ursa.rule{luabind, objs, ursa.util.system_template{"ar rcs $TARGET $SOURCES"}}
