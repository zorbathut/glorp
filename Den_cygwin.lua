require "glorp/Den_util"

local params = ...

local rv = {}

token_literal("CC", params.glop.cc)
token_literal("CXXFLAGS", "-mwindows -DWIN32 -Iglorp/glop/release/cygwin/include")
token_literal("LDFLAGS", "-L/lib/mingw -Lglorp/glop/release/cygwin/lib -mwindows -lopengl32 -lmingw32 -lwinmm -lkernel32 -luser32 -lgdi32 -lwinspool -lcomdlg32 -ladvapi32 -lshell32 -lole32 -loleaut32 -luuid -lodbc32 -lodbccp32 -ldinput -ldxguid -lglu32 -lws2_32 -limagehlp -lpsapi")

token_literal("FLAC", "\"/cygdrive/c/Program\ Files\ \(x86\)/FLAC/flac.exe\"")

token_literal("LUA_FLAGS", "")

rv.extension = ".exe"
rv.lua_buildtype = "cygwin"

-- runnable
rv.create_runnable = function(dat)
  local libpath = "glorp/glop/Glop/third_party/system_cygwin/dll"
  local libs = "fmodex.dll"
  local liboutpath = params.builddir

  local dlls = {}
  for libname in (libs):gmatch("[^%s]+") do
    table.insert(dlls, ursa.rule{("%s%s"):format(liboutpath, libname), ("%s/%s"):format(libpath, libname), ursa.util.copy{}})
  end
  
  return {deps = {dlls, dat.mainprog}, cli = ("%s%s.exe"):format(params.builddir, params.name)}
end

-- installers
function rv.installers()
  -- first we have to build the entire path layout
  local data = {}

  -- DLLs and executables
  table.insert(data, ursa.rule{params.builddir .. "deploy/" .. params.name .. ".exe", params.builddir .. params.name .. ".exe", ursa.util.system_template{"cp $SOURCE $TARGET && strip -s $TARGET"}})
  table.insert(data, ursa.rule{params.builddir .. "deploy/data/reporter.exe", params.builddir .. "reporter.exe", ursa.util.system_template{"cp $SOURCE $TARGET && strip -s $TARGET"}})
  table.insert(data, ursa.rule{params.builddir .. "deploy/fmodex.dll", params.builddir .. "fmodex.dll", ursa.util.copy{}})
  table.insert(data, ursa.rule{params.builddir .. "deploy/licenses.txt", "glorp/resources/licenses.txt", ursa.util.copy{}})

  -- second we generate our actual data copies
  ursa.token.rule{"built_data", "#datafiles", function ()
    local items = {}
    for _, v in pairs(ursa.token{"datafiles"}) do
      table.insert(items, ursa.rule{(params.builddir .. "deploy/%s"):format(v.dst), v.src, ursa.util.system_template{v.cli}})
    end
    return items
  end, always_rebuild = true}
  
  cull_data(params.builddir .. "deploy/", {data})

  ursa.token.rule{"installers", {data, "#built_data", "#culled_data", "#version"}, function ()
    local v = ursa.token{"version"}
    
    local exesuffix = ("%s-%s.exe"):format(params.midname, v)
    local exedest = "build/" .. exesuffix
    ursa.rule{params.builddir .. "installer.nsi", {data, ursa.util.token_deferred{"built_data"}, "#culled_data", "#version", "glorp/installer.nsi.template"}, function()
      local files = ursa.system{("cd %sdeploy && find . -type f | sed s*\\\\./**"):format(params.builddir)}
      local dir = ursa.system{("cd %sdeploy && find . -type d | sed s*\\\\./**"):format(params.builddir)}
      
      local inp = io.open("glorp/installer.nsi.template", "rb")
      local otp = io.open(params.builddir .. "installer.nsi", "w")
      local function outwrite(txt)
        otp:write(txt .. "\n")
      end
      
      local install, uninstall = "", ""
      
      for line in dir:gmatch("[^\n]+") do
        line = line:gsub("/", "\\")
        install = install .. ('CreateDirectory "$INSTDIR\\%s"\n'):format(line)
        uninstall = ('RMDir "$INSTDIR\\%s"\n'):format(line) .. uninstall
      end
      
      for line in files:gmatch("[^\n]+") do
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
          outwrite(('OutFile ../%s'):format(exesuffix))
        else
          outwrite(line:gsub("$$$LONGNAME$$%$", params.longname):gsub("$$$EXENAME$$%$", params.name .. ".exe"))
        end
      end
      
      inp:close()
      otp:close()
    end}
    
    return {
      ursa.rule{("build/%s-%s.zip"):format(params.midname, v), {data, ursa.util.token_deferred{"built_data"}, "#culled_data", "#version"}, ursa.util.system_template{"cd #builddir/deploy ; zip -9 -r ../../../$TARGET *"}},
      ursa.rule{exedest, {data, ursa.util.token_deferred{"built_data"}, "#culled_data", "#version", params.builddir .. "installer.nsi"}, ursa.util.system_template{"cd #builddir && /cygdrive/c/Program\\ Files\\ \\(x86\\)/NSIS/makensis.exe installer.nsi"}},
    }
  end, always_rebuild = true}
  
  return ursa.util.token_deferred{"installers"}
end

return rv
