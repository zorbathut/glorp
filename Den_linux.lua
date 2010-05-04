require "glorp/Den_util"

local params = ...

local rv = {}

ursa.token.rule{"FLAC", nil, function () return "flac" end}

rv.extension = ".prog"  -- have to use something or it'll conflict

token_literal("CC", params.glop.cc)
token_literal("CXXFLAGS", "-m32 -DLINUX -Iglorp/Glop/release/linux/include")
token_literal("LDFLAGS", "-m32 -lGL -lGLU")

token_literal("LUA_FLAGS", "-DLUA_USE_LINUX -m32")

rv.lua_buildtype = "linux"

local runnable_deps

rv.create_runnable = function(dat)
  local libpath = "glorp/Glop/Glop/third_party/system_linux/lib"
  local libs = "libfmodex.so"
  local liboutpath = params.builddir

  local dlls = {}
  for libname in (libs):gmatch("[^%s]+") do
    table.insert(dlls, ursa.rule{("%s%s"):format(liboutpath, libname), ("%s/%s"):format(libpath, libname), ursa.util.copy{}})
  end
  
  return {deps = {dlls, dat.mainprog}, cli = ("%s%s.prog"):format(params.builddir, params.name)}
end

-- installers
function rv.installers()
  -- first we have to build the entire path layout
  local data = {}
  
  local datadir = params.builddir .. "deploy/" .. params.midname .. "/"

  -- DLLs and executables
  local stripped = ursa.rule{params.builddir .. params.name .. ".prog.stripped", params.builddir .. params.name .. ".prog", ursa.util.system_template{"cp $SOURCE $TARGET && strip -s $TARGET"}}
  table.insert(data, ursa.rule{datadir .. params.name, stripped, function ()
    print("install_name_tool fakeout ugliness")
    local ni = io.open(params.builddir .. params.name .. ".prog.stripped", "rb")
    local dat = ni:read("*a")
    ni:close()
    
    local orig = "glorp/Glop/release/linux/lib/libfmodex.so"
    local new = "data/libfmodex.so"
    assert(#new <= #orig)
    while #new < #orig do
      new = new .. "\0"
    end
    
    local ndat = dat:gsub(orig, new, 1)
    
    assert(#dat == #ndat)
    
    assert(not ndat:find(orig))
    
    local out = io.open(datadir .. params.name, "wb")
    out:write(ndat)
    out:close()
    
    ursa.system{"chmod +x " .. datadir .. params.name}
  end})
  table.insert(data, ursa.rule{datadir .. "data/reporter", params.builddir .. "reporter.prog", ursa.util.system_template{"cp $SOURCE $TARGET && strip -s $TARGET"}})
  table.insert(data, ursa.rule{datadir .. "data/libfmodex.so", params.builddir .. "libfmodex.so", ursa.util.copy{}})
  table.insert(data, ursa.rule{datadir .. "data/licenses.txt", "glorp/resources/licenses.txt", ursa.util.copy{}})

  -- second we generate our actual data copies
  ursa.token.rule{"built_data", "#datafiles", function ()
    local items = {}
    for _, v in pairs(ursa.token{"datafiles"}) do
      table.insert(items, ursa.rule{(datadir .. "%s"):format(v.dst), v.src, ursa.util.system_template{v.cli}})
    end
    return items
  end, always_rebuild = true}
  
  cull_data(datadir, {data})

  local installers = {}
  
  table.insert(installers, ursa.rule{("build/%s-%s.tgz"):format(params.midname, ursa.token{"version"}), {data, ursa.util.token_deferred{"built_data"}, "#culled_data", "#version"}, ursa.util.system_template{("cd %sdeploy && tar -pczf ../../%s-%s.tgz \"%s\""):format(params.builddir, params.midname, ursa.token{"version"}, params.midname)}})
  
  return installers
end

return rv
