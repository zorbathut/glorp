local params = ...

local headers = params.headers
local libs = params.libs
local platform = params.platform
local builddir = params.builddir
local addHeadersFor = params.addHeadersFor

local libjpeg_dir = "glorp/libs/jpeg-8c"

ursa.token.rule{"libjpeg files", nil, ("cd %s && find . -type f | sed s*\\\\./**"):format(libjpeg_dir)}  -- we are assuming this isn't changing

local libjpeg_files = {}
for file in ursa.token{"libjpeg files"}:gmatch("([^\n]+)") do
  table.insert(libjpeg_files, ursa.rule{builddir .. "lib_build/libjpeg/" .. file, libjpeg_dir .. "/" .. file, ursa.util.copy{}})
end

local makefile = ursa.rule{{builddir .. "lib_build/libjpeg/Makefile", builddir .. "lib_build/libjpeg/jconfig.h"}, libjpeg_files, ursa.util.system_template{('cd %s/lib_build/libjpeg && CC=#CC CFLAGS="#CCFLAGS" LDFLAGS="#LDFLAGS" ./configure --disable-dependency-tracking'):format(builddir)}}

local lib = ursa.rule{builddir .. "lib_build/libjpeg/.libs/libjpeg.a", {makefile, libjpeg_files}, ('cd %s/lib_build/libjpeg && make clean && make -j5'):format(builddir)}

-- now we copy shit to the right place
libs.libjpeg = ursa.rule{builddir .. "lib_release/lib/libjpeg.a", lib, ursa.util.copy{}}
headers.libjpeg = {}
table.insert(headers.libjpeg, ursa.rule{builddir .. "lib_release/include/jpeglib/jpeglib.h", builddir .. "lib_build/libjpeg/jpeglib.h", ursa.util.copy{}})
table.insert(headers.libjpeg, ursa.rule{builddir .. "lib_release/include/jpeglib/jconfig.h", builddir .. "lib_build/libjpeg/jconfig.h", ursa.util.copy{}})
table.insert(headers.libjpeg, ursa.rule{builddir .. "lib_release/include/jpeglib/jmorecfg.h", builddir .. "lib_build/libjpeg/jmorecfg.h", ursa.util.copy{}})