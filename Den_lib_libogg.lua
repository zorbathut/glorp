local params = ...

local headers = params.headers
local libs = params.libs
local platform = params.platform
local builddir = params.builddir

headers.libogg = {}
local path = "glorp/libs/libogg-1.2.2"

local files = {}

ursa.token.rule{"libogg_files", nil, function () return ursa.system{("cd %s && find . -type f | sed s*\\\\./**"):format(path)} end}
for item in ursa.token{"libogg_files"}:gmatch("([^\n]+)") do
  if item ~= "libogg.spec" then
    table.insert(files, ursa.rule{builddir .. "lib_build/libogg/" .. item, path .. "/" .. item, ursa.util.copy{}})
  end
  
  if item:match("include/ogg/.*%.h$") then
    table.insert(headers.libogg, ursa.rule{builddir .. "lib_release/" .. item, path .. "/" .. item, ursa.util.copy{}})
  end
end

local makefile = ursa.rule{{builddir .. "lib_build/libogg/Makefile", builddir .. "lib_build/libogg/include/ogg/config_types.h"}, files, ursa.util.system_template{('cd %slib_build/libogg && CC="#CC" CFLAGS="#CCFLAGS" LDFLAGS="#LDFLAGS" ./configure --enable-shared=no --enable-static=yes --disable-dependency-tracking'):format(builddir, ursa.token{"CC"}, libcflags, libldflags)}}

local lib = ursa.rule{builddir .. "lib_build/libogg/src/.libs/libogg.a", makefile, ('cd %slib_build/libogg && make -j1'):format(builddir)}

libs.libogg = ursa.rule{builddir .. "lib_release/lib/libogg.a", lib, ursa.util.copy{}}

table.insert(headers.libogg, ursa.rule{builddir .. "lib_release/include/ogg/config_types.h", builddir .. "lib_build/libogg/include/ogg/config_types.h", ursa.util.copy{}})