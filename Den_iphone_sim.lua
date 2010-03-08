require "glorp/Den_util"

local params = ...

local rv = {}

local ipp = loadfile("glorp/Den_util_iphone.lua")(params, rv)

token_literal("CC", "/Developer/Platforms/iPhoneSimulator.platform/Developer/usr/bin/g++")
token_literal("CXXFLAGS", ipp.cxx .. " -arch i386 -isysroot /Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator3.0.sdk -mmacosx-version-min=10.5  -Fglorp/Glop/build/Glop -Iglorp/Glop/Glop/OSX")
token_literal("LDFLAGS", ipp.ld .. " -arch i386 -isysroot /Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator3.0.sdk -mmacosx-version-min=10.5 -Fglorp/Glop/build/Glop")

token_literal("LUA_FLAGS", "-arch i386 -isysroot /Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator3.0.sdk -D__IPHONE_OS_VERSION_MIN_REQUIRED=30000")

rv.no_cli_params = true

local runnable_deps

rv.create_runnable = function(dat)
  local basepath = "build/iphone_sim/" .. params.longname .. ".app"
  
  local runnable = {}
  
  local current_glop = params.glop.lib
  local current_glop_iteration = 0
  
  -- copy our main executable while tweaking the path
  table.insert(runnable, ursa.rule{basepath .. "/" .. params.longname, dat.mainprog, ursa.util.system_template{"cp $SOURCE $TARGET && install_name_tool -change build/Glop/Glop.framework/Glop @executable_path/Glop $TARGET"}})
  
  -- copy the main glop library
  table.insert(runnable, ursa.rule{basepath .. "/Glop", current_glop, ursa.util.system_template{"cp $SOURCE $TARGET"}})
  
  -- info.plist
  table.insert(runnable, ursa.rule{basepath .. "/Info.plist", "#version", function ()
    print("Writing info.plist")
    local pl = io.open(basepath .. "/Info.plist", "w")
    
    pl:write([[
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>CFBundleDevelopmentRegion</key>
  <string>English</string>
  <key>CFBundleExecutable</key>
  <string>]] .. params.longname .. [[</string>
  <key>CFBundleIconFile</key>
  <string>mandible.icns</string>
  <key>CFBundleIdentifier</key>
  <string>com.mandible-games.]] .. params.name .. [[.iphone_sim</string>
  <key>CFBundleName</key>
  <string>]] .. params.longname .. [[</string>
  <key>CFBundlePackageType</key>
  <string>APPL</string>
  <key>CFBundleVersion</key>
  <string>]] .. ursa.token{"version"} .. [[</string>
	<key>LSRequiresIPhoneOS</key>
	<true/>
</dict>
</plist>]])
    
    pl:close()
    
  end})
  
  -- we need to do data copies in the build for this target. TODO: some kind of low-build high-build option?
  ursa.token.rule{"built_data", "#datafiles", function ()
    local items = {}
    for _, v in pairs(ursa.token{"datafiles"}) do
      table.insert(items, ursa.rule{basepath .. "/" .. v.dst, v.src, ursa.util.system_template{v.cli}})
    end
    --assert(false)
    return items
  end, always_rebuild = true}
  
  runnable_deps = runnable
  
  return {deps = {runnable, ursa.util.token_deferred{"built_data"}}, cli = 'killall "iPhone Simulator" ; glorp/resources/iphonesim launch `pwd`"/' .. basepath .. '"'}
end

-- doesn't even make sense for the iphone sim
function rv.installers()
  return {}
end

return rv
