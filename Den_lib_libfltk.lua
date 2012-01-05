local params = ...

local headers = params.headers
local libs = params.libs
local platform = params.platform
local builddir = params.builddir

if platform == "linux" then
  -- now let's fltk it up a notch
  headers.libfltk = {}
  local path = "glorp/libs/fltk-1.1.10"
  
  local files = {}
  
  ursa.token.rule{"fltk_files", nil, function () return ursa.system{("cd %s && find . -type f | sed s*\\\\./**"):format(path)} end}
  for item in ursa.token{"fltk_files"}:gmatch("([^\n]+)") do
    if item == "config.h" or item == "fltk.spec" or item == "fltk.list" or item == "fltk-config" then
    elseif item == "configure" then
      table.insert(files, ursa.rule{builddir .. "lib_build/fltk/" .. item, path .. "/" .. item, ursa.util.system_template{([[sed
        -e "s/-lXext//"
        
        $SOURCE > $TARGET && chmod +x $TARGET]]):format(ursa.token{"CC"}):gsub("\n", " ")}})
    else
      table.insert(files, ursa.rule{builddir .. "lib_build/fltk/" .. item, path .. "/" .. item, ursa.util.copy{}})
    end
    
    
    if item:match("^FL.*") then
      table.insert(headers.libfltk, ursa.rule{builddir .. "lib_release/include/" .. item, path .. "/" .. item, ursa.util.copy{}})
    end
  end
  
  local makefile = ursa.rule{builddir .. "lib_build/fltk/FL/Makefile", {files, libs.zlib, libs.libpng, libs.libjpeg}, ursa.util.system_template{('cd %slib_build/fltk && CC="#CC" CFLAGS="#CCFLAGS" CPPFLAGS="#CXXFLAGS" LDFLAGS="#LDFLAGS -L/usr/lib" ./configure --disable-localjpeg --disable-localzlib --disable-localpng'):format(builddir)}}
  
  local lib = ursa.rule{builddir .. "lib_build/fltk/lib/libfltk.a", makefile, ('cd %slib_build/fltk && make'):format(builddir)}
  
  libs.libfltk = ursa.rule{builddir .. "lib_release/lib/libfltk.a", lib, ursa.util.copy{}}
else
  assert(false)
end
