
local params, rv = ...

loadfile("glorp/Den_util_osx.lua")(params, rv)

rv.noluajit = true
rv.gles = true

rv.build_overrides = {}
function rv.build_overrides.wav(dst)
  return "/usr/bin/afconvert -f caff -d ima4 $SOURCE $TARGET", dst:gsub("%.wav", ".caf")
end
function rv.build_overrides.flac(dst)
  local rngnum = math.random()
  return ("#FLAC -d $SOURCE -o /tmp/%d.intermed.wav && /usr/bin/afconvert -f caff -d ima4 /tmp/%d.intermed.wav $TARGET && rm /tmp/%d.intermed.wav"):format(rngnum, rngnum, rngnum), dst:gsub("%.flac", ".caf")  -- dunno if this will work
end

return {cxx = "-D__IPHONE_OS_VERSION_MIN_REQUIRED=30000 -Fglorp/Glop/build/Glop -DIPHONE", ld = "-Lglorp/Glop/Glop/OSX/lib -framework Foundation -framework UIKit -framework OpenGLES -framework QuartzCore"}
