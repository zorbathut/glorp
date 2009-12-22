local params = ...

-- first we have to build the entire path layout
local data = {}

-- DLLs and executables
for _, file in ipairs({params.name .. ".exe", "reporter.exe"}) do
  table.insert(data, ursa.rule{"build/deploy/" .. file, "build/" .. file, ursa.util.system_template{"cp $SOURCE $TARGET && strip -s $TARGET"}})
end
for _, file in ipairs({"fmodex.dll", "libfreetype-6.dll", "libpng-3.dll"}) do
  table.insert(data, ursa.rule{"build/deploy/" .. file, "build/" .. file, ursa.util.system_template{"cp $SOURCE $TARGET"}})
end
table.insert(data, ursa.rule{"build/deploy/licenses.txt", "glorp/resources/licenses.txt", ursa.util.system_template{"cp $SOURCE $TARGET"}})

-- generic copy-a-lot framework
local function copy_a_lot(token, destprefix, sourceprefix)
  ursa.token.rule{token .. "_copy", "#" .. token .. "_files", function ()
      local data_items = {}
      for k in ursa.token{token .. "_files"}:gmatch("[^%s]+") do
        local ext = k:match("^.*%.([^%.]+)$")
        
        local dst = "build/deploy/" .. destprefix .. k
        local src = sourceprefix .. k
        
        if ext == "png" then
          table.insert(data_items, ursa.rule{dst, src, ursa.util.system_template{"pngcrush -brute -rem alla -cc $SOURCE $TARGET"}})
        elseif ext == "wav" then
          dst = dst:gsub("%.wav", ".ogg")
          table.insert(data_items, ursa.rule{dst, src, ursa.util.system_template{"oggenc --downmix -q 5 -o $TARGET $SOURCE || oggenc -q 6 -o $TARGET $SOURCE"}})
        else
          table.insert(data_items, ursa.rule{dst, src, ursa.util.system_template{"cp $SOURCE $TARGET"}})
        end
      end
      return data_items
    end, always_rebuild = true}
  return ursa.util.token_deferred{token .. "_copy"}
end

-- replicate the data structure in
ursa.token.rule{"data_files", nil, "cd data && find -type f | sed s*\\\\./**", always_rebuild = true}
table.insert(data, copy_a_lot("data", "data/", "data/"))

ursa.token.rule{"stock_files", nil, "cd glorp/resources && ls mandible_games.png", always_rebuild = true}
table.insert(data, copy_a_lot("stock", "data/", "glorp/resources/"))

-- JIT lua files
ursa.token.rule{"jit_files", nil, "cd glorp/resources/jit && ls"}
table.insert(data, copy_a_lot("jit", "data/jit/", "glorp/resources/jit/"))

-- Lua files in Glorp
ursa.token.rule{"luaglorp_files", nil, "cd glorp && ls *.lua | grep -v Den"}
table.insert(data, copy_a_lot("luaglorp", "data/", "glorp/"))

-- Lua files and font files in our core
ursa.token.rule{"core_files", nil, "ls *.lua *.ttf"}
table.insert(data, copy_a_lot("core", "", ""))

  
ursa.token.rule{"data", data, ""}


ursa.token.rule{"installers", {data, "#version"}, function ()
  local v = ursa.token{"version"}
  
  local exesuffix = ("%s-%s.exe"):format(params.name, v)
  local exedest = "build/" .. exesuffix
  print("ED:", exedest)
  ursa.rule{"build/installer.nsi", {data, "glorp/installer.nsi.template"}, function(dst, src)
    local files = ursa.util.system{"cd build/deploy && find -type f | sed s*\\\\./**"}
    local dir = ursa.util.system{"cd build/deploy && find -type d | sed s*\\\\./**"}
    
    local inp = io.open("glorp/installer.nsi.template", "rb")
    local otp = io.open("build/installer.nsi", "w")
    local function outwrite(txt)
      print(txt)
      otp:write(txt .. "\n")
    end
    
    local install, uninstall = "", ""
    
    for line in dir:gmatch("[^%s]+") do
      line = line:gsub("/", "\\")
      install = install .. ('CreateDirectory "$INSTDIR\\%s"\n'):format(line)
      uninstall = ('RMDir "$INSTDIR\\%s"\n'):format(line) .. uninstall
    end
    
    for line in files:gmatch("[^%s]+") do
      line = line:gsub("/", "\\")
      install = install .. ('File "/oname=%s" "%s"\n'):format(line, "deploy\\" .. line)
      uninstall = ('Delete "$INSTDIR\\%s"\n'):format(line) .. uninstall
    end
    
    for line in inp:lines() do
      if line == "$$$INSTALL$$$" then
        outwrite(install)
      elseif line == "$$$UNINSTALL$$$" then
        outwrite(uninstall)
      elseif line == "$$$VERSION$$$" then
        outwrite(('!define PRODUCT_VERSION "%s"'):format(v))
      elseif line == "$$$TYPE$$$" then
        outwrite('!define PRODUCT_TYPE "release"')
      elseif line == "$$$OUTFILE$$$" then
        outwrite(('OutFile %s'):format(exesuffix))
      else
        outwrite(line:gsub("$$$LONGNAME$$%$", params.longname):gsub("$$$EXENAME$$%$", params.name .. ".exe"))
      end
    end
    
    inp:close()
    otp:close()
  end}
  
  return {
    ursa.rule{("build/%s-%s.zip"):format(params.name, v), data, ursa.util.system_template{"cd build/deploy ; zip -9 -r ../../$TARGET *"}},
    ursa.rule{exedest, "build/installer.nsi", "cd build && /cygdrive/c/Program\\ Files\\ \\(x86\\)/NSIS/makensis.exe installer.nsi"},
  }
end, always_rebuild = true}

local installers = {}


return ursa.util.token_deferred{"installers"}
