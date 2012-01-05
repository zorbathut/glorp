local params = ...

local headers = params.headers
local libs = params.libs
local platform = params.platform
local builddir = params.builddir
local addHeadersFor = params.addHeadersFor

addHeadersFor("lua")

-- now let's lfs it up a notch
headers.lfs = {}
local path = "glorp/libs/luafilesystem"

local files = {}

ursa.token.rule{"lfs_files", nil, function () return ursa.system{("cd %s && find . -type f | sed s*\\\\./**"):format(path)} end}
for item in ursa.token{"lfs_files"}:gmatch("([^\n]+)") do
  table.insert(files, ursa.rule{builddir .. "lib_build/lfs/" .. item, path .. "/" .. item, ursa.util.copy{}})
end

table.insert(headers.lfs, ursa.rule{builddir .. "lib_release/include/lfs.h", path .. "/src/lfs.h", ursa.util.copy{}})

local lib = ursa.rule{builddir .. "lib_build/lfs/liblfs.a", {files, headers.lua}, ursa.util.system_template{('cd %slib_build/lfs && %s %s %s -c src/lfs.c -o src/lfs.o && ar rcs liblfs.a src/lfs.o'):format(builddir, ursa.token{"CC"}, libcflags, libldflags)}}

libs.lfs = ursa.rule{builddir .. "lib_release/lib/liblfs.a", lib, ursa.util.copy{}}