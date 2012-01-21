local params = ...

local headers = params.headers
local libs = params.libs
local platform = params.platform
local builddir = params.builddir

local results = ursa.embed{"glorp/libs/frames", "Den_embed", {{cxx = ursa.token{"CXX"}, cxxflags = ursa.token{"CXXFLAGS"}, build = "../../../" .. builddir .. "lib_build/frames", prefix = "../../../" .. builddir .. "lib_release"}}}

headers.frames = results.headers
libs.frames = results.lib