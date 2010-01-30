
local params, rv = ...

loadfile("glorp/Den_util_osx.lua")(params, rv)

rv.noluajit = true
