require "glorp/Den_util"

local params = ...

local rv = {}

loadfile("glorp/Den_util_osx.lua")(params, rv)

token_literal("CC", params.glop.cc)
token_literal("CXXFLAGS", "-isysroot /Developer/SDKs/MacOSX10.5.sdk -mmacosx-version-min=10.5 -arch i386 -DMACOSX -Iglorp/Glop/release/osx/include")
token_literal("LDFLAGS", "-isysroot /Developer/SDKs/MacOSX10.5.sdk -mmacosx-version-min=10.5 -arch i386 -framework OpenGL -framework Carbon -framework AGL -framework ApplicationServices -framework IOKit -framework AppKit")

token_literal("LUA_FLAGS", "-isysroot /Developer/SDKs/MacOSX10.5.sdk -mmacosx-version-min=10.5 -DLUA_USE_LINUX -arch i386")

rv.lua_buildtype = "macosx"

local runnable_deps

rv.create_runnable = function(dat)
  local basepath = "build/osx/" .. params.longname .. ".app"
  
  local runnable = {}
  
  local current_glop = params.glop.lib
  local current_glop_iteration = 0
  
  -- copy our main executable
  table.insert(runnable, ursa.rule{basepath .. "/Contents/MacOS/" .. params.longname, dat.mainprog, ursa.util.system_template{("cp $SOURCE $TARGET && install_name_tool -change ./libfmodex.dylib @executable_path/../Frameworks/libfmodex.dylib $TARGET"):format(cli)}})
  
  --[[
  local function tweak_glop(cli)
    current_glop_iteration = current_glop_iteration + 1
    current_glop = ursa.rule{"build/osx/glop_lib_" .. current_glop_iteration, current_glop, ursa.util.system_template{("cp $SOURCE $TARGET && install_name_tool %s $TARGET"):format(cli)}}
    --assert(false)
  end]]
  
  -- copy subsidiary libraries
  for libname in ("libfmodex.dylib"):gmatch("[^%s]+") do
    table.insert(runnable, ursa.rule{("%s/Contents/Frameworks/%s"):format(basepath, libname), ("glorp/Glop/Glop/third_party/system_osx/lib/%s"):format(libname), ursa.util.copy{}})
    --tweak_glop(("-change ./%s @executable_path/../Frameworks/%s"):format(libname, libname))
  end
  
  -- copy the main glop library
  --table.insert(runnable, ursa.rule{basepath .. "/Contents/Frameworks/Glop.framework/Glop", current_glop, ursa.util.system_template{"cp $SOURCE $TARGET"}})
  
  runnable_deps = runnable
  
  return {deps = runnable, cli = '"' .. basepath .."/Contents/MacOS/" .. params.longname .. '"'}
end

-- installers
function rv.installers()
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
  table.insert(binaries, ursa.rule{app_prefix .. "Contents/Resources/data/reporter", params.builddir .. "reporter.prog", ursa.util.system_template{"strip -S -x -o $TARGET $SOURCE"}})

  -- here's our bootstrapper for sane version errors
  table.insert(binaries, ursa.rule{app_prefix .. "Contents/MacOS/" .. params.longname .. "-SystemVersionCheck", "glorp/resources/SystemVersionCheck", ursa.util.copy{}})
  
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
  <key>CFBundleTypeIconFile</key>
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
      <string>10.5.0</string>
  </dict>
</dict>
</plist>]])
    
    pl:close()
    
  end}
  
  cull_data(app_prefix, {binaries, icon, infoplist})
  
  return ursa.rule{("build/%s.dmg"):format(ursa.token{"outputprefix"}), {binaries, ursa.util.token_deferred{"built_data"}, "#culled_data", infoplist, icon}, ursa.util.system_template{('hdiutil create -srcfolder "build/osx/deploy/%s.app" $TARGET -ov'):format(params.longname)}}
end

return rv
