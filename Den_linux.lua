local params = ...

local rv = {}

ursa.token.rule{"FLAC", nil, function () return "flac" end}

rv.extension = ".prog"  -- have to use something or it'll conflict

token_literal("CC", params.glop.cc)
token_literal("CXXFLAGS", "-m32 -DLINUX -Iglorp/glop/release/linux/include")
token_literal("LDFLAGS", "-m32 -lGL -lGLU -lrt")

token_literal("LUA_FLAGS", "-DLUA_USE_LINUX -m32")

rv.lua_buildtype = "linux"

local runnable_deps

rv.create_runnable = function(dat)
  local libpath = "glorp/glop/Glop/third_party/system_linux/lib"
  local libs = "libfmodex.so"
  local liboutpath = params.builddir

  local dlls = {}
  for libname in (libs):gmatch("[^%s]+") do
    table.insert(dlls, ursa.rule{("%s%s"):format(liboutpath, libname), ("%s/%s"):format(libpath, libname), ursa.util.copy{}})
  end
  
  return {deps = {dlls, dat.mainprog, "build/linux/glorp/constants.lua"}, cli = ("%s%s.prog"):format(params.builddir, params.name)}
end

rv.appprefix = params.builddir .. "deploy/" .. params.midname .. "/"
rv.dataprefix = rv.appprefix
  
-- installers
function rv.installers()
  -- first we have to build the entire path layout
  local data = {}

  -- DLLs and executables
  local stripped = ursa.rule{params.builddir .. params.name .. ".prog.stripped", params.builddir .. params.name .. ".prog", ursa.util.system_template{"cp $SOURCE $TARGET && strip -s $TARGET"}}
  table.insert(data, ursa.rule{rv.dataprefix .. params.midname, stripped, function ()
    print("install_name_tool fakeout ugliness")
    local ni = io.open(params.builddir .. params.name .. ".prog.stripped", "rb")
    local dat = ni:read("*a")
    ni:close()
    
    local orig = "glorp/glop/release/linux/lib/libfmodex.so"
    local new = "data/libfmodex.so"
    assert(#new <= #orig)
    while #new < #orig do
      new = new .. "\0"
    end
    
    local ndat = dat:gsub(orig, new, 1)
    
    assert(#dat == #ndat)
    
    assert(not ndat:find(orig))
    
    local out = io.open(rv.dataprefix .. params.midname, "wb")
    out:write(ndat)
    out:close()
    
    ursa.system{"chmod +x " .. rv.dataprefix .. params.midname}
  end})
  table.insert(data, ursa.rule{rv.dataprefix .. "data/reporter", params.builddir .. "reporter.prog", ursa.util.system_template{"cp $SOURCE $TARGET && strip -s $TARGET"}})
  table.insert(data, ursa.rule{rv.dataprefix .. "data/libfmodex.so", params.builddir .. "libfmodex.so", ursa.util.system_template{"cp $SOURCE $TARGET && (execstack -c $TARGET || /usr/sbin/execstack -c $TARGET)"}})
  
  local dfn = {}
  for i = 0, 2 do
    table.insert(dfn, rv.dataprefix .. "data/mandible_icon-" .. i .. ".png")
  end
  table.insert(data, ursa.rule{dfn, "glorp/resources/mandicomulti.ico", ursa.util.system_template{"convert $SOURCE " .. rv.dataprefix .. "data/mandible_icon.png"}})
  
  cull_data({data})

  local installers = {}
  
  table.insert(installers, ursa.rule{("build/%s-%s.tgz"):format(params.midname, ursa.token{"version"}), {data, ursa.util.token_deferred{"built_data"}, "#culled_data", "#version"}, ursa.util.system_template{("cd %sdeploy && tar -pczf ../../%s-%s.tgz \"%s\""):format(params.builddir, params.midname, ursa.token{"version"}, params.midname)}})
  
  return installers
end

return rv
