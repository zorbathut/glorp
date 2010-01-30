require "glorp/Den_util"

local params = ...

local rv = {}

loadfile("glorp/Den_util_iphone.lua")(params, rv)

token_literal("CC", "/Developer/Platforms/iPhoneSimulator.platform/Developer/usr/bin/g++")
token_literal("CXXFLAGS", "-D__IPHONE_OS_VERSION_MIN_REQUIRED=30000 -arch i386 -isysroot /Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator3.0.sdk -mmacosx-version-min=10.5")
token_literal("LDFLAGS", "-arch i386 -isysroot /Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator3.0.sdk -mmacosx-version-min=10.5 -framework Foundation -framework UIKit -framework OpenGLES -framework QuartzCore")

token_literal("LUA_FLAGS", "")

local runnable_deps

rv.create_runnable = function(dat)
  local basepath = "build/iphone_sim/" .. params.longname .. ".app"
  
  local runnable = {}
  
  local current_glop = params.glop.lib
  local current_glop_iteration = 0
  
  -- copy our main executable
  table.insert(runnable, ursa.rule{basepath .. "/" .. params.longname, dat.mainprog, ursa.util.system_template{"cp $SOURCE $TARGET"}})
  
  runnable_deps = runnable
  
  return {deps = runnable, cli = 'glorp/iphonesim launch "`pwd`' .. basepath .. '"'}
end

-- doesn't even make sense for the iphone sim
function rv.installers()
  return {}
end

return rv
