local params = ...

local headers = params.headers
local libs = params.libs
local platform = params.platform
local builddir = params.builddir

-- this is mostly abandoned

-- now let's box2d it up a notch
headers.box2d = {}
local path = "glorp/libs/Box2D_v2.1.2"

local files = {}

ursa.token.rule{"box2d_files", nil, function () return ursa.system{("cd %s && find . -type f | sed s*\\\\./**"):format(path)} end}
for item in ursa.token{"box2d_files"}:gmatch("([^\n]+)") do
  if item == "Build/Readme.txt" then
  else
    table.insert(files, ursa.rule{builddir .. "lib_build/box2d/" .. item, path .. "/" .. item, ursa.util.copy{}})
  end
  
  if item:match("%.h$") then
    table.insert(headers.box2d, ursa.rule{builddir .. "lib_release/include/" .. item, path .. "/" .. item, ursa.util.copy{}})
  end
end

local sedreplace = ""
if platform == "cygwin" then
  -- cmake makes this very hard
  sedreplace = " && " .. ("sed -i -e s@/usr/bin/gcc.exe@%s@g -e s@/usr/bin/c++.exe@%s@g -e s@/cygdrive/c@c:@g `find . -type f`"):format(ursa.token{"CC"}, ursa.token{"CC"})
end

local lib = ursa.rule{builddir .. "lib_build/box2d/Build/Box2D/libBox2D.a", {files}, ursa.util.system_template{('cd %slib_build/box2d/Build && CFLAGS="#CCFLAGS" CXXFLAGS="#CXXFLAGS" LDFLAGS="#LDFLAGS -L/usr/lib" cmake -DBOX2D_INSTALL=OFF -DBOX2D_INSTALL_DOC=OFF -DBOX2D_BUILD_EXAMPLES=OFF -DCMAKE_BUILD_TYPE=Release ..' .. sedreplace .. ' && make && (rmdir c\\: || true)'):format(builddir)}}

libs.box2d = ursa.rule{builddir .. "lib_release/lib/libbox2d.a", lib, ursa.util.copy{}}
