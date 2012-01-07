local params = ...

token_literal("CC", "gcc")
token_literal("CCFLAGS", "-isysroot /Developer/SDKs/MacOSX10.5.sdk -mmacosx-version-min=10.5 -arch i386")

token_literal("CXX", "g++")
token_literal("CXXFLAGS", "-isysroot /Developer/SDKs/MacOSX10.5.sdk -mmacosx-version-min=10.5 -arch i386")

token_literal("LDFLAGS", "-isysroot /Developer/SDKs/MacOSX10.5.sdk -mmacosx-version-min=10.5 -arch i386")

token_literal("FINALCXXFLAGS", "-DMACOSX")
token_literal("FINALLDFLAGS", "-framework OpenGL -framework Carbon -framework AGL -framework ApplicationServices -framework IOKit -framework AppKit")

token_literal("FLAC", "flac")

token_literal("LUA_FLAGS", "-isysroot /Developer/SDKs/MacOSX10.5.sdk -mmacosx-version-min=10.5 -DLUA_USE_LINUX -arch i386")

local rv = {}

rv.extension = ".prog"  -- have to use something or it'll conflict
rv.lua_buildtype = "macosx"

local basepath = "build/osx/" .. params.longname .. ".app"

local runnable_deps

rv.create_runnable = function(dat)  
  local runnable = {}

  -- copy our main executable
  table.insert(runnable, ursa.rule{basepath .. "/Contents/MacOS/" .. params.longname, dat.mainprog, ursa.util.system_template{("cp $SOURCE $TARGET && install_name_tool -change /usr/local/lib/libportaudio.2.dylib @executable_path/../Frameworks/libportaudio.2.dylib -change #pwd/build/osx/lib_build/opengalsoft/build/libopengal.1.dylib @executable_path/../Frameworks/libopengal.1.dylib -add_rpath @executable_path/../Frameworks $TARGET"):format(basepath)}})
  
  table.insert(runnable, ursa.rule{("%s/Contents/Frameworks/libportaudio.2.dylib"):format(basepath), "build/osx/lib_release/lib/libportaudio.dylib", ursa.util.copy{}})
  table.insert(runnable, ursa.rule{("%s/Contents/Frameworks/libopengal.1.dylib"):format(basepath), "build/osx/lib_release/lib/libopengal.dylib", ursa.util.copy{}})
  
  runnable_deps = runnable
  
  return {deps = {runnable}, cli = '"' .. basepath .."/Contents/MacOS/" .. params.longname .. '"'}
end

rv.appprefix = ("build/osx/deploy/%s.app/"):format(params.longname)
rv.dataprefix = rv.appprefix .. "Contents/Resources/"

-- installers
function rv.installers()
  assert(runnable_deps)
  
  local binaries = {}
  
  -- first we mirror our run structure over, plus stripping (oh baby oh baby)
  for k, v in pairs(ursa.relative_from{runnable_deps}) do
    local sufix = v:match(("build/osx/[-%s '.]*.app/(.*)"):format(params.longname))
    assert(sufix)
    
    table.insert(binaries, ursa.rule{rv.appprefix .. sufix, v, ursa.util.system_template{"strip -S -x -o $TARGET $SOURCE"}})
  end
  
  -- copy the reporter
  table.insert(binaries, ursa.rule{rv.appprefix .. "Contents/Resources/data/reporter", params.builddir .. "reporter.prog", ursa.util.system_template{"strip -S -x -o $TARGET $SOURCE"}})

  -- here's our bootstrapper for sane version errors
  table.insert(binaries, ursa.rule{rv.appprefix .. "Contents/MacOS/" .. params.longname .. "-SystemVersionCheck", "glorp/resources/osx/SystemVersionCheck", ursa.util.copy{}})

  local icon = ursa.rule{rv.appprefix .. "Contents/Resources/mandible.icns", "glorp/resources/osx/mandicon.png", ursa.util.system_template{("glorp/resources/osx/makeicns -in $SOURCE -out $TARGET")}}
  
  local infoplist = ursa.rule{rv.appprefix .. "Contents/Info.plist", "#version", function ()
    print("Writing info.plist")
    local pl = io.open(rv.appprefix .. "Contents/Info.plist", "w")
    
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
  
  cull_data({binaries, icon, infoplist})
  
  return ursa.rule{("build/%s.dmg"):format(ursa.token{"outputprefix"}), {binaries, ursa.util.token_deferred{"built_data"}, "#culled_data", infoplist, icon}, ursa.util.system_template{('hdiutil create -srcfolder "build/osx/deploy/%s.app" $TARGET -ov'):format(params.longname)}}
end

return rv
