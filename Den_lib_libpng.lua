local params = ...

local headers = params.headers
local libs = params.libs
local platform = params.platform
local builddir = params.builddir
local addHeadersFor = params.addHeadersFor

addHeadersFor("zlib")

local libpng_dir = "glorp/libs/libpng-1.4.3"

--ursa.token.rule{"libpng files", nil, ("cd %s && find . -type f | sed s*\\\\./**"):format(libpng_dir)}  -- we are assuming this isn't changing
ursa.token.rule{"libpng files", nil, ("cd %s && find . -maxdepth 1 -type f | sed s*\\\\./**"):format(libpng_dir)}  -- we are assuming this isn't changing

local libpng_files = {}
for file in ursa.token{"libpng files"}:gmatch("([^\n]+)") do
  table.insert(libpng_files, ursa.rule{builddir .. "lib_build/libpng/" .. file, libpng_dir .. "/" .. file, ursa.util.copy{}})
end

local makefile = ursa.rule{{builddir .. "lib_build/libpng/Makefile"}, {libpng_files, headers.zlib, libs.zlib}, ursa.util.system_template{('cd %s/lib_build/libpng && CC=#CC CFLAGS="#CCFLAGS" LDFLAGS="#LDFLAGS" ./configure --disable-dependency-tracking'):format(builddir)}}

local lib = ursa.rule{builddir .. "/lib_build/libpng/.libs/libpng14.a", {makefile, libpng_files}, ('cd %s/lib_build/libpng && make -j5'):format(builddir)}

-- now we copy shit to the right place
libs.libpng = ursa.rule{builddir .. "/lib_release/lib/libpng.a", lib, ursa.util.copy{}}
headers.libpng = {}
table.insert(headers.libpng, ursa.rule{builddir .. "/lib_release/include/png.h", builddir .. "lib_build/libpng/png.h", ursa.util.copy{}})
table.insert(headers.libpng, ursa.rule{builddir.. "/lib_release/include/pngconf.h", builddir .. "lib_build/libpng/pngconf.h", ursa.util.copy{}})