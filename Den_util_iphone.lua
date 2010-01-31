
local params, rv = ...

loadfile("glorp/Den_util_osx.lua")(params, rv)

rv.noluajit = true
rv.gles = true
rv.nocurl = true

return {cxx = "-D__IPHONE_OS_VERSION_MIN_REQUIRED=30000 -arch i386 -isysroot /Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator3.0.sdk -mmacosx-version-min=10.5 -Fglorp/Glop/build/Glop -DIPHONE", ld = "-Lglorp/Glop/Glop/OSX/lib -arch i386 -isysroot /Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator3.0.sdk -mmacosx-version-min=10.5 -framework Foundation -framework UIKit -framework OpenGLES -framework QuartzCore"}
