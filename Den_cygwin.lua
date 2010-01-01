require "glorp/Den_util"

local params = ...

local rv = {}

rv.cxx_flags = "-mno-cygwin -mwindows -DWIN32 -DCURL_STATICLIB -I/usr/mingw/local/include/boost-1_38_0 -Iglorp/glop/build/Glop/local/include"
rv.ld_flags = "-L/lib/mingw -L/usr/mingw/local/lib -Lglorp/glop/Glop/cygwin/lib -mno-cygwin -mwindows -lopengl32 -lmingw32 -lwinmm -lkernel32 -luser32 -lgdi32 -lwinspool -lcomdlg32 -ladvapi32 -lshell32 -lole32 -loleaut32 -luuid -lodbc32 -lodbccp32 -ldinput -ldxguid -lglu32 -lws2_32 -ljpeg -lfreetype -lz"
rv.extension = ".exe"

-- runnable
rv.create_runnable = function(dat)
  local libpath = "glorp/Glop/Glop/cygwin/dll"
  local libs = "libfreetype-6.dll fmodex.dll libpng-3.dll"
  local liboutpath = "build/"

  local dlls = {}
  for libname in (libs):gmatch("[^%s]+") do
    table.insert(dlls, ursa.rule{("%s%s"):format(liboutpath, libname), ("%s/%s"):format(libpath, libname), ursa.util.system_template{"cp $SOURCE $TARGET"}})
  end
  
  return {deps = {dlls, dat.mainprog}, cli = ("build/%s.exe"):format(params.name)}
  -- more to come
end

-- installers
function rv.installers()
  -- first we have to build the entire path layout
  local data = {}

  -- DLLs and executables
  for _, file in ipairs({params.name .. ".exe", "reporter.exe"}) do
    table.insert(data, ursa.rule{"build/deploy/" .. file, "build/" .. file, ursa.util.system_template{"cp $SOURCE $TARGET && strip -s $TARGET"}})
  end
  for _, file in ipairs({"fmodex.dll", "libfreetype-6.dll", "libpng-3.dll"}) do
    table.insert(data, ursa.rule{"build/deploy/" .. file, "build/" .. file, ursa.util.system_template{"cp $SOURCE $TARGET"}})
  end
  table.insert(data, ursa.rule{"build/deploy/libGlop.dll", params.glop.lib, ursa.util.system_template{"cp $SOURCE $TARGET"}})
  table.insert(data, ursa.rule{"build/deploy/licenses.txt", "glorp/resources/licenses.txt", ursa.util.system_template{"cp $SOURCE $TARGET"}})

  -- second we generate our actual data copies
  ursa.token.rule{"built_data", "#datafiles", function ()
    local items = {}
    for _, v in pairs(ursa.token{"datafiles"}) do
      table.insert(items, ursa.rule{("build/deploy/%s"):format(v.dst), v.src, ursa.util.system_template{v.cli}})
    end
    return items
  end, always_rebuild = true}
  
  cull_data("build/deploy/", {data})

  ursa.token.rule{"installers", {data, ursa.util.token_deferred{"built_data"}, "#culled_data", "#version"}, function ()
    local v = ursa.token{"version"}
    
    local exesuffix = ("%s-%s.exe"):format(params.midname, v)
    local exedest = "build/" .. exesuffix
    ursa.rule{"build/installer.nsi", {data, "glorp/installer.nsi.template"}, function(dst, src)
      local files = ursa.util.system{"cd build/deploy && find . -type f | sed s*\\\\./**"}
      local dir = ursa.util.system{"cd build/deploy && find . -type d | sed s*\\\\./**"}
      
      local inp = io.open("glorp/installer.nsi.template", "rb")
      local otp = io.open("build/installer.nsi", "w")
      local function outwrite(txt)
        otp:write(txt .. "\n")
      end
      
      local install, uninstall = "", ""
      
      for line in dir:gmatch("[^%s]+") do
        line = line:gsub("/", "\\")
        install = install .. ('CreateDirectory "$INSTDIR\\%s"\n'):format(line)
        uninstall = ('RMDir "$INSTDIR\\%s"\n'):format(line) .. uninstall
      end
      
      for line in files:gmatch("[^%s]+") do
        line = line:gsub("/", "\\")
        install = install .. ('File "/oname=%s" "%s"\n'):format(line, "deploy\\" .. line)
        uninstall = ('Delete "$INSTDIR\\%s"\n'):format(line) .. uninstall
      end
      
      for line in inp:lines() do
        if line == "$$$INSTALL$$$" then
          outwrite(install)
        elseif line == "$$$UNINSTALL$$$" then
          outwrite(uninstall)
        elseif line == "$$$VERSION$$$" then
          outwrite(('!define PRODUCT_VERSION "%s"'):format(v))
        elseif line == "$$$TYPE$$$" then
          outwrite('!define PRODUCT_TYPE "release"')
        elseif line == "$$$OUTFILE$$$" then
          outwrite(('OutFile %s'):format(exesuffix))
        else
          outwrite(line:gsub("$$$LONGNAME$$%$", params.longname):gsub("$$$EXENAME$$%$", params.name .. ".exe"))
        end
      end
      
      inp:close()
      otp:close()
    end}
    
    return {
      ursa.rule{("build/%s-%s.zip"):format(params.midname, v), {data, ursa.util.token_deferred{"built_data"}, "#culled_data", "#version"}, ursa.util.system_template{"cd build/deploy ; zip -9 -r ../../$TARGET *"}},
      ursa.rule{exedest, "build/installer.nsi", "cd build && /cygdrive/c/Program\\ Files\\ \\(x86\\)/NSIS/makensis.exe installer.nsi"},
    }
  end, always_rebuild = true}
  
  return ursa.util.token_deferred{"installers"}
end

return rv
