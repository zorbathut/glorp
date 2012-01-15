local params = ...

local headers = params.headers
local libs = params.libs
local platform = params.platform
local builddir = params.builddir
local addHeadersFor = params.addHeadersFor

-- I suppose we can opengal it up a notch
headers.opengal = {}
local path = "glorp/opengal32"

local files = {}

ursa.token.rule{"opengal_files", nil, function () return ursa.system{("cd %s && find . -type f | sed s*\\\\./**"):format(path)} end}
for item in ursa.token{"opengal_files"}:gmatch("([^\n]+)") do
  table.insert(files, ursa.rule{builddir .. "lib_build/opengalsoft/" .. item, path .. "/" .. item, ursa.util.copy{}})
  
  if item:match("include/.*%.h$") then
    table.insert(headers.opengal, ursa.rule{builddir .. "lib_release/" .. item, path .. "/" .. item, ursa.util.copy{}})
  end
end

local lib
if platform == "cygwin" then
  addHeadersFor("dx")

  -- cmake makes this very hard
  local sedreplace = " && " .. ("sed -i -e s@/usr/bin/gcc.exe@%s.exe@g -e s@/usr/bin/c++.exe@%s.exe@g -e s@/cygdrive/c@c:@g -e \"s@enable-auto-import@enable-auto-import %s@\" `find . -type f | grep -v empty`"):format(ursa.token{"CC"}, ursa.token{"CC"}, ursa.token{"LDFLAGS"})
  local opengalcflags = ursa.token{"CCFLAGS"} .. " -I" .. ursa.token{"PWD"} .. "/build/lib_release/include"
  local lib = ursa.rule{{builddir .. "lib_build/opengalsoft/build/libOpenGAL32.dll.a", builddir .. "lib_build/opengalsoft/build/OpenGAL32-1.dll"}, {files, headers.dx}, ursa.util.system_template{('cd %slib_build/opengalsoft/build && CFLAGS="%s" CXXFLAGS="%s" cmake -DCMAKE_BUILD_TYPE=Release -DALSA=OFF -DSOLARIS=OFF -DOSS=OFF -DWINMM=OFF -DPORTAUDIO=OFF -DPULSEAUDIO=OFF -DEXAMPLES=OFF -DDLOPEN=OFF -DEXTRA_LIBS=winmm ..' .. sedreplace .. ' && make && (rmdir c\\: || true)'):format(builddir, opengalcflags, opengalcflags)}}
  
  libs.opengal = ursa.rule{builddir .. "lib_release/lib/libOpenGAL32.dll.a", builddir .. "lib_build/opengalsoft/build/libOpenGAL32.dll.a", ursa.util.copy{}}
  ursa.rule{builddir .. "lib_release/bin/OpenGAL32-1.dll", builddir .. "lib_build/opengalsoft/build/OpenGAL32-1.dll", ursa.util.copy{}}
else
  local ext = "so"
  local po = "OFF"
  if platform == "osx" then
    ext = "dylib"
    po = "ON"
  end
  
  local lib = ursa.rule{builddir .. "lib_build/opengalsoft/build/libopengal." .. ext, {files, headers.portaudio, libs.portaudio}, ursa.util.system_template{('cd %slib_build/opengalsoft/build && CFLAGS="#CCFLAGS -I%s/build/%s/lib_release/include" CXXFLAGS="#CXXFLAGS -I%s/build/%s/lib_release/include" LDFLAGS="#LDFLAGS -L/usr/lib -L%s/build/%s/lib_release/lib" cmake -DCMAKE_BUILD_TYPE=Release DSOLARIS=OFF -DOSS=OFF -DWINMM=OFF -DPORTAUDIO=%s -DPULSEAUDIO=OFF -DEXAMPLES=OFF .. && make'):format(builddir, ursa.token{"PWD"}, platform, ursa.token{"PWD"}, platform, ursa.token{"PWD"}, platform, po)}}
  
  if platform == "osx" then
    libs.opengal = ursa.rule{builddir .. "lib_release/lib/libopengal.dylib", builddir .. "lib_build/opengalsoft/build/libopengal.dylib", ursa.util.copy{}}
    ursa.rule{builddir .. "lib_release/bin/libopengal.dylib", builddir .. "lib_build/opengalsoft/build/libopengal.dylib", ursa.util.copy{}}
  else
    libs.opengal = ursa.rule{builddir .. "lib_release/lib/libopengal.so", builddir .. "lib_build/opengalsoft/build/libopengal.so", ursa.util.copy{}}
    ursa.rule{builddir .. "lib_release/bin/libopengal.so", builddir .. "lib_build/opengalsoft/build/libopengal.so", ursa.util.copy{}}
  end
end
