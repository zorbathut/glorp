local params = ...

local headers = params.headers
local libs = params.libs
local platform = params.platform
local builddir = params.builddir
local addHeadersFor = params.addHeadersFor

local libfreetype_dir = "glorp/libs/freetype-2.3.12"
  
--ursa.token.rule{"libfreetype files", nil, ("cd %s && find . -type f | sed s*\\\\./**"):format(libfreetype_dir)}  -- we are assuming this isn't changing
ursa.token.rule{"libfreetype files", nil, ("cd %s && ((find . -type f | grep -v builds | grep -v docs | sed s*\\\\./**) && find builds/ -maxdepth 1 -type f && find builds/unix/ -type f)"):format(libfreetype_dir)}  -- we are assuming this isn't changing

local libfreetype_files = {}
headers.libfreetype = {}
for file in ursa.token{"libfreetype files"}:gmatch("([^\n]+)") do
  if file == "builds/unix/unix-def.in" then
    table.insert(libfreetype_files, ursa.rule{builddir .. "lib_build/libfreetype/" .. file, libfreetype_dir .. "/" .. file, ursa.util.system_template{'sed "s/TOP_DIR := .*/TOP_DIR := ./" $SOURCE > $TARGET'}})
  else
    table.insert(libfreetype_files, ursa.rule{builddir .. "lib_build/libfreetype/" .. file, libfreetype_dir .. "/" .. file, ursa.util.copy{}})
  end
  
  if file:match("^include.*") then
    table.insert(headers.libfreetype, ursa.rule{builddir .. "lib_release/include/" .. file:match("include/(.*)"), libfreetype_dir .. "/" .. file, ursa.util.copy{}})
  end
end

local makefile = ursa.rule{{builddir .. "lib_build/libfreetype/builds/unix/freetype-config"}, libfreetype_files, ursa.util.system_template{('cd %s/lib_build/libfreetype && CC=#CC CFLAGS="#CCFLAGS" LDFLAGS="#LDFLAGS" ./configure'):format(builddir)}}

local lib = ursa.rule{builddir .. "lib_build/libfreetype/objs/.libs/libfreetype.a", {makefile, libfreetype_files}, ('cd %s/lib_build/libfreetype && make -j5'):format(builddir)}

-- now we copy shit to the right place
libs.libfreetype = ursa.rule{builddir .. "lib_release/lib/libfreetype.a", lib, ursa.util.copy{}}