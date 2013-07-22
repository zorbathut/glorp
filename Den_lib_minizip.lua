local params = ...

local headers = params.headers
local libs = params.libs
local platform = params.platform
local builddir = params.builddir
local addHeadersFor = params.addHeadersFor

addHeadersFor("zlib")

local minizip_dir = "glorp/libs/zlib-1.2.3/contrib/minizip"

token_literal("minizip files", "unzip.h unzip.c ioapi.h ioapi.c crypt.h")

local minizip_files = {}
for file in ursa.token{"minizip files"}:gmatch("([^ ]+)") do
  table.insert(minizip_files, ursa.rule{builddir .. "lib_build/minizip/" .. file, minizip_dir .. "/" .. file, ursa.util.copy{}})
end

local lib = ursa.rule{builddir .. "lib_build/minizip/minizip.a", {minizip_files, headers.zlib, libs.zlib}, ('cd %s/lib_build/minizip && gcc -c unzip.c -o unzip.o && gcc -c ioapi.c -o ioapi.o && ar rcs minizip.a unzip.o ioapi.o'):format(builddir)}

-- now we copy shit to the right place
libs.minizip = ursa.rule{builddir .. "lib_release/lib/libminizip.a", lib, ursa.util.copy{}}
headers.minizip = {}
table.insert(headers.minizip, ursa.rule{builddir .. "lib_release/include/unzip.h", builddir .. "lib_build/minizip/unzip.h", ursa.util.copy{}})
table.insert(headers.minizip, ursa.rule{builddir .. "lib_release/include/ioapi.h", builddir .. "lib_build/minizip/ioapi.h", ursa.util.copy{}})