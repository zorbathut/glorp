local params = ...

local headers = params.headers
local libs = params.libs
local platform = params.platform
local builddir = params.builddir
local addHeadersFor = params.addHeadersFor

addHeadersFor("libogg")

headers.libvorbis = {}
local path = "glorp/libs/libvorbis-1.3.2"

local files = {}

ursa.token.rule{"libvorbis_files", nil, function () return ursa.system{("cd %s && find . -type f | sed s*\\\\./**"):format(path)} end}
for item in ursa.token{"libvorbis_files"}:gmatch("([^\n]+)") do
  if item ~= "libvorbis.spec" then
    table.insert(files, ursa.rule{builddir .. "lib_build/libvorbis/" .. item, path .. "/" .. item, ursa.util.copy{}})
  end
  
  if item:match("include/vorbis/.*%.h$") then
    table.insert(headers.libvorbis, ursa.rule{builddir .. "lib_release/" .. item, path .. "/" .. item, ursa.util.copy{}})
  end
end

local makefile = ursa.rule{builddir .. "lib_build/libvorbis/Makefile", {files, headers.libogg, libs.libogg}, ursa.util.system_template{('cd %slib_build/libvorbis && CC="#CC" CFLAGS="#CCFLAGS" CPPFLAGS="#CCFLAGS -I%s/build/%s/lib_release/include" LDFLAGS="#LDFLAGS -L%s/build/%s/lib_release/lib" ./configure --enable-shared=no --enable-static=yes --disable-oggtest'):format(builddir, ursa.token{"PWD"}, platform, ursa.token{"PWD"}, platform)}}

ursa.rule{{builddir .. "lib_build/libvorbis/lib/.libs/libvorbis.a", builddir .. "lib_build/libvorbis/lib/.libs/libvorbisfile.a"}, makefile, ('cd %slib_build/libvorbis && make -j1'):format(builddir)}

libs.libvorbis = {ursa.rule{builddir .. "lib_release/lib/libvorbis.a", builddir .. "lib_build/libvorbis/lib/.libs/libvorbis.a", ursa.util.copy{}}, ursa.rule{builddir .. "lib_release/lib/libvorbisfile.a", builddir .. "lib_build/libvorbis/lib/.libs/libvorbisfile.a", ursa.util.copy{}}}
