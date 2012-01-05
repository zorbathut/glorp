local params = ...

local headers = params.headers
local libs = params.libs
local platform = params.platform
local builddir = params.builddir

headers.libflac = {}
local path = "glorp/libs/flac-1.2.1"

local files = {}

ursa.token.rule{"libflac_files", nil, function () return ursa.system{("cd %s && find . -type f | sed s*\\\\./**"):format(path)} end}
for item in ursa.token{"libflac_files"}:gmatch("([^\n]+)") do
  if item == "include/share/alloc.h" then
    table.insert(files, ursa.rule{builddir .. "lib_build/libflac/" .. item, path .. "/" .. item, ursa.util.system_template{([[sed
      -e "s/#  ifdef _MSC_VER/#  if 1 == 1/"
      
      $SOURCE > $TARGET]]):gsub("\n", " ")}})
  elseif item == "src/libFLAC/ia32/nasm.h" then
    table.insert(files, ursa.rule{builddir .. "lib_build/libflac/" .. item, path .. "/" .. item, ursa.util.system_template{([[sed
      -e "s/%error unsupported object format!/%define FLAC__PUBLIC_NEEDS_UNDERSCORE/"
      
      $SOURCE > $TARGET]]):gsub("\n", " ")}})
  else
    table.insert(files, ursa.rule{builddir .. "lib_build/libflac/" .. item, path .. "/" .. item, ursa.util.copy{}})
  end
  
  if item:match("include/FLAC/.*%.h$") then
    table.insert(headers.libflac, ursa.rule{builddir .. "lib_release/" .. item, path .. "/" .. item, ursa.util.copy{}})
  end
end

local ldflags2 = ""
local cflags2 = ""

if platform == "osx" then
  cflags2 = " -m32"
  ldflags2 = " -m32"
end

local makefile = ursa.rule{builddir .. "lib_build/libflac/Makefile", files, ursa.util.system_template{('cd %slib_build/libflac && CC="#CC" CFLAGS="#CCFLAGS %s" CXXFLAGS="#CXXFLAGS %s" LDFLAGS="#LDFLAGS %s" ./configure --enable-shared=no --enable-static=yes --disable-ogg --disable-xmms-plugin --disable-cpplibs --disable-thorough-tests'):format(builddir, cflags2, cflags2, ldflags2)}}

libs.libflac = ursa.rule{builddir .. "lib_build/libflac/src/libFLAC/.libs/libFLAC.a", makefile, ('cd %slib_build/libflac && make'):format(builddir)}