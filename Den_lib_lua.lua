local params = ...

local headers = params.headers
local libs = params.libs
local platform = params.platform
local builddir = params.builddir

local lua_exec  -- we're not really using this right now

headers.lua = {}

if not params.noluajit then
  local path = "glorp/libs/LuaJIT-2.0.0-beta5-p1/"
  ursa.token.rule{"lua_files", "!" .. path, function () return ursa.system{("cd %s && find . -type f | sed s*\\\\./**"):format(path)} end}
  local copied = {}
  for k in ursa.token{"lua_files"}:gmatch("([^\n]+)") do
    if k == "src/Makefile" then
      -- let's make some changes
      table.insert(copied, ursa.rule{builddir .. "lib_build/lua/" .. k, path .. k, ursa.util.system_template{([[
        sed
          -e "s@CC= gcc@CC= %s@"
          -e "s/.XCFLAGS+= -DLUA_USE_APICHECK/XCFLAGS+= -DLUA_USE_APICHECK/"
          -e "s/.XCFLAGS+= -DLUA_USE_ASSERT/XCFLAGS+= -DLUA_USE_ASSERT/"
          -e "s/.BUILDMODE= static/BUILDMODE= static/"
          -e "s/.CC= gcc -m32/CC= gcc -m32/" 
          -e "s/HOST_RM= del/# HOST_RM= del/" ]] ..
          
          -- -e "s/.XCFLAGS+= -DLUAJIT_DISABLE_JIT/XCFLAGS+= -DLUAJIT_DISABLE_JIT/"  --[[ This segment should be commented out if you want the jit to actually work! ]] ..
        [[$SOURCE > $TARGET]]):format(ursa.token{"CC"}):gsub("\n", " ")}})
    elseif k:match("src/buildvm_.*%.h") then
    else
      table.insert(copied, ursa.rule{builddir .. "lib_build/lua/" .. k, path .. k, ursa.util.copy{}})
    end
  end
  
  lua_exec = builddir .. "lib_build/lua/src/luajit"
  if platform == "cygwin" then
    lua_exec = lua_exec .. ".exe"
  end
  
  ursa.rule{{builddir .. "lib_build/lua/src/libluajit.a", lua_exec}, copied, ursa.util.system_template{("cd %slib_build/lua && make -j5"):format(builddir)}}
  ursa.rule{builddir .. "lib_release/lib/liblua.a", builddir .. "lib_build/lua/src/libluajit.a", ursa.util.copy{}}
  
  -- luajit includes it in src
  table.insert(headers.lua, ursa.rule{builddir .. "lib_release/include/lua.hpp", builddir .. "lib_build/lua/src/lua.hpp", ursa.util.copy{}})
  table.insert(headers.lua, ursa.rule{builddir .. "lib_release/include/luajit.h", builddir .. "lib_build/lua/src/luajit.h", ursa.util.copy{}})
else
  local files = "lapi lcode ldebug ldo ldump lfunc lgc llex lmem lobject lopcodes lparser lstate lstring ltable ltm lundump lzio lcoco linit lauxlib lbaselib ldblib liolib lmathlib loslib ltablib lstrlib loadlib lvm"
  local path = "glorp/libs/lua-5.1.4/"
  
  ursa.token.rule{"lua_files", "!" .. path, function () return ursa.system{("cd %s && find src etc -type f"):format(path)} end}
  local copied = {}
  for k in ursa.token{"lua_files"}:gmatch("([^\n]+)") do
    table.insert(copied, ursa.rule{builddir .. "lib_build/lua/" .. k, path .. k, ursa.util.copy{}})
  end
  
  local luaobjects = {}
  for k in files:gmatch("([^ ]+)") do  -- yes okay hardcoded shut up
    table.insert(luaobjects, ursa.rule{builddir .. "lib_build/lua/src/" .. k .. ".o", copied, ursa.util.system_template{"#CC -O3 -fomit-frame-pointer #LUA_FLAGS -I#builddir/lib_build/lua/dynasm -Wall -x c -c -o $TARGET #builddir/lib_build/lua/src/" .. k .. ".c"}})
  end
  
  lua_exec = "lua" -- isn't actually our copy of lua, we'll make this work better later I suppose
  
  ursa.rule{builddir .. "lib_release/lib/liblua.a", luaobjects, ursa.util.system_template{"ar rcs $TARGET $SOURCES"}} -- wonder if this'll work right under cygwin
  
  -- base lua includes it in etc
  table.insert(headers.lua, ursa.rule{builddir .. "lib_release/include/lua.hpp", builddir .. "lib_build/lua/etc/lua.hpp", ursa.util.copy{}})
end

libs.lua = {builddir .. "lib_release/lib/liblua.a"}

for k in ("lua.h lauxlib.h lualib.h luaconf.h"):gmatch("([^ ]+)") do
  table.insert(headers.lua, ursa.rule{builddir .. "lib_release/include/" .. k, builddir .. "lib_build/lua/src/" .. k, ursa.util.copy{}})
end
