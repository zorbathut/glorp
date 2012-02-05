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

local results = ursa.embed{"glorp/libs/frames", "Den_embed", {{cxx = ursa.token{"CXX"}, cxxflags = "-I../../../" .. builddir .. "lib_release/include " .. ursa.token{"CXXFLAGS"}, build = "../../../" .. builddir .. "lib_build/frames", prefix = "../../../" .. builddir .. "lib_release", dependencies = {headers.boost, headers.glew, headers.libjpeg, headers.libpng, headers.zlib}}}}

headers.frames = results.headers
libs.frames = results.lib