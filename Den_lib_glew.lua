local params = ...

local headers = params.headers
local libs = params.libs
local platform = params.platform
local builddir = params.builddir
local addHeadersFor = params.addHeadersFor

local glew_dir = "glorp/libs/glew-1.7.0"

ursa.token.rule{"glew files", nil, ("cd %s/include && find . -type f | sed s*\\\\./**"):format(glew_dir)}  -- we are assuming this isn't changing

headers.glew = {}
for file in ursa.token{"glew files"}:gmatch("([^\n]+)") do
  table.insert(headers.glew, ursa.rule{builddir .. "lib_release/include/" .. file, glew_dir .. "/include/" .. file, ursa.util.system_template{([[sed
      -e "s/ifdef GLEW_STATIC/if 1/"
      
      $SOURCE > $TARGET && chmod +x $TARGET]]):format():gsub("\n", " ")}})
end

local obj = ursa.rule{builddir .. "lib_build/glew/glew.o", {glew_dir .. "/src/glew.c", headers.glew}, ursa.util.system_template{("#CC -O3 -DGLEW_STATIC #CCFLAGS -I#BUILDDIR/lib_release/include -c -o $TARGET %s"):format(glew_dir .. "/src/glew.c")}}

libs.glew = ursa.rule{builddir .. "lib_release/lib/libglew.a", obj, ursa.util.system_template{"ar rcs $TARGET $SOURCES"}}
