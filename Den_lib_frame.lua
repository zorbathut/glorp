local params = ...

local headers = params.headers
local libs = params.libs
local platform = params.platform
local builddir = params.builddir
local addHeadersFor = params.addHeadersFor

addHeadersFor("boost")
addHeadersFor("glew")
addHeadersFor("libjpeg")
addHeadersFor("libpng")
addHeadersFor("zlib")
addHeadersFor("libfreetype")
addHeadersFor("lua")

local results = ursa.embed{"glorp/libs/frame", "Den_embed", {{cxx = ursa.token{"CXX"}, cxxflags = "-I../../../" .. builddir .. "lib_release/include " .. ursa.token{"CXXFLAGS"}, build = "../../../" .. builddir .. "lib_build/frame", prefix = "../../../" .. builddir .. "lib_release", dependencies = {headers.boost, headers.glew, headers.libjpeg, headers.libpng, headers.zlib, headers.libfreetype, headers.lua}}}}

headers.frame = results.headers
libs.frame = results.lib