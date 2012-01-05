local params = ...

local headers = params.headers
local libs = params.libs
local platform = params.platform
local builddir = params.builddir

local zlib_dir = "glorp/libs/zlib-1.2.3"
  
--ursa.token.rule{"zlib files", nil, ("cd %s && find . -type f | sed s*\\\\./**"):format(zlib_dir)}  -- we are assuming this isn't changing
ursa.token.rule{"zlib files", nil, ("cd %s && find . -maxdepth 1 -type f | sed s*\\\\./**"):format(zlib_dir)}  -- we are assuming this isn't changing
local zlib_files = {}
for file in ursa.token{"zlib files"}:gmatch("([^\n]+)") do
  table.insert(zlib_files, ursa.rule{builddir .. "lib_build/zlib/" .. file, zlib_dir .. "/" .. file, ursa.util.copy{}})
end

local makefile = ursa.rule{{builddir .. "lib_build/zlib/Makefile", builddir .. "lib_build/zlib/zconf.h"}, zlib_files, ursa.util.system_template{('cd %s/lib_build/zlib && CC=#CC CFLAGS="#CCFLAGS" LDFLAGS="#LDFLAGS" ./configure'):format(builddir)}}

local lib = ursa.rule{builddir .. "lib_build/zlib/libz.a", {makefile, zlib_files}, ('cd %s/lib_build/zlib && make -j5'):format(builddir)}

-- now we copy shit to the right place
libs.zlib = ursa.rule{builddir .. "lib_release/lib/libz.a", lib, ursa.util.copy{}}
headers.zlib = {}
table.insert(headers.zlib, ursa.rule{builddir .. "lib_release/include/zlib.h", builddir .. "lib_build/zlib/zlib.h", ursa.util.copy{}})
table.insert(headers.zlib, ursa.rule{builddir .. "lib_release/include/zconf.h", builddir .. "lib_build/zlib/zconf.h", ursa.util.copy{}})