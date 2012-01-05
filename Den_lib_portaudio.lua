local params = ...

local headers = params.headers
local libs = params.libs
local platform = params.platform
local builddir = params.builddir

local lua_exec  -- we're not really using this right now

if platform == "osx" then
  headers.portaudio = {}
  local path = "glorp/libs/portaudio"
  
  local files = {}
  
  ursa.token.rule{"portaudio_files", nil, function () return ursa.system{("cd %s && find . -type f | sed s*\\\\./**"):format(path)} end}
  for item in ursa.token{"portaudio_files"}:gmatch("([^\n]+)") do
    table.insert(files, ursa.rule{builddir .. "lib_build/portaudio/" .. item, path .. "/" .. item, ursa.util.copy{}})
    
    if item:match("include/.*%.h$") then
      table.insert(headers.portaudio, ursa.rule{builddir .. "lib_release/" .. item, path .. "/" .. item, ursa.util.copy{}})
    end
  end
  
  local makefile = ursa.rule{builddir .. "lib_build/portaudio/Makefile", files, ursa.util.system_template{('cd %slib_build/portaudio && CC="#CC" CFLAGS="#CCFLAGS" LDFLAGS="#LDFLAGS" ./configure --enable-shared --disable-static --with-alsa=no --with-jack=no --disable-mac-universal'):format(builddir)}}
  
  local lib = ursa.rule{builddir .. "lib_build/portaudio/lib/.libs/libportaudio.dylib", makefile, ('cd %slib_build/portaudio && make'):format(builddir)}
  
  libs.portaudio = ursa.rule{builddir .. "lib_release/lib/libportaudio.dylib", lib, ursa.util.copy{}}
else
  headers.portaudio = {}
  libs.portaudio = {}
end
