require "glorp/Den_util"

local params = ...

local rv = {}

ursa.token.rule{"FLAC", nil, function () return "flac" end}

rv.extension = ".prog"  -- have to use something or it'll conflict

token_literal("CC", params.glop.cc)
token_literal("CXXFLAGS", "-m32 -DLINUX -I/opt/local/include -Iglorp/Glop/release/linux/include")
token_literal("LDFLAGS", "-m32 -lGL -lGLU")

token_literal("LUA_FLAGS", "-DLUA_USE_LINUX -m32")

rv.lua_buildtype = "linux"

local runnable_deps

rv.create_runnable = function(dat)
  local libpath = "glorp/Glop/Glop/third_party/system_linux/lib"
  local libs = "libfmodex.so"
  local liboutpath = params.builddir

  local dlls = {}
  for libname in (libs):gmatch("[^%s]+") do
    table.insert(dlls, ursa.rule{("%s%s"):format(liboutpath, libname), ("%s/%s"):format(libpath, libname), ursa.util.copy{}})
  end
  
  return {deps = {dlls, dat.mainprog}, cli = ("%s%s.prog"):format(params.builddir, params.name)}
end

-- installers
function rv.installers()
  return {}
  --[=[
  assert(runnable_deps)
  local app_prefix = ("build/osx/deploy/%s.app/"):format(params.longname)
  
  local binaries = {}
  
  -- first we mirror our run structure over, plus stripping (oh baby oh baby)
  for k, v in pairs(ursa.relative_from{runnable_deps}) do
    local sufix = v:match(("build/osx/[-%s '.]*.app/(.*)"):format(params.longname))
    assert(sufix)
    
    table.insert(binaries, ursa.rule{app_prefix .. sufix, v, ursa.util.system_template{"strip -S -x -o $TARGET $SOURCE"}})
  end
  
  -- copy the reporter
  table.insert(binaries, ursa.rule{app_prefix .. "Contents/Resources/reporter", params.builddir .. "reporter.prog", ursa.util.system_template{"strip -S -x -o $TARGET $SOURCE"}})

  -- here's our bootstrapper for sane version errors
  table.insert(binaries, ursa.rule{app_prefix .. "Contents/MacOS/" .. params.longname .. "-SystemVersionCheck", "glorp/resources/SystemVersionCheck", ursa.util.system_template{"cp $SOURCE $TARGET"}})
  
  -- second we generate our actual data copies
  ursa.token.rule{"built_data", "#datafiles", function ()
    local items = {}
    for _, v in pairs(ursa.token{"datafiles"}) do
      table.insert(items, ursa.rule{app_prefix .. "Contents/Resources/" .. v.dst, v.src, ursa.util.system_template{v.cli}})
    end
    --assert(false)
    return items
  end, always_rebuild = true}

  
  local icon = ursa.rule{app_prefix .. "Contents/Resources/mandible.icns", "glorp/resources/mandicon.png", ursa.util.system_template{("glorp/resources/makeicns -in $SOURCE -out $TARGET")}}
  
  local infoplist = ursa.rule{app_prefix .. "Contents/Info.plist", "#version", function ()
    print("Writing info.plist")
    local pl = io.open(app_prefix .. "Contents/Info.plist", "w")
    
    pl:write([[
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>CFBundleDevelopmentRegion</key>
  <string>English</string>
  <key>CFBundleExecutable</key>
  <string>]] .. params.longname .. [[-SystemVersionCheck</string>
  <key>CFBundleIconFile</key>
  <string>mandible.icns</string>
  <key>CFBundleIdentifier</key>
  <string>com.mandible-games.]] .. params.name .. [[</string>
  <key>CFBundleInfoDictionaryVersion</key>
  <string>6.0</string>
  <key>CFBundleName</key>
  <string>]] .. params.longname .. [[</string>
  <key>CFBundlePackageType</key>
  <string>APPL</string>
  <key>CFBundleVersion</key>
  <string>]] .. ursa.token{"version"} .. [[</string>
  <key>LSEnvironment</key>
  <dict>
      <key>MinimumSystemVersion</key>
      <string>10.6.0</string>
  </dict>
</dict>
</plist>]])
    
    pl:close()
    
  end}
  
  cull_data(app_prefix, {binaries, icon, infoplist})
  
  return ursa.rule{("build/%s.dmg"):format(ursa.token{"outputprefix"}), {binaries, ursa.util.token_deferred{"built_data"}, "#culled_data", infoplist, icon}, ursa.util.system_template{('hdiutil create -srcfolder "build/osx/deploy/%s.app" $TARGET -ov'):format(params.longname)}}]=]
end

return rv
