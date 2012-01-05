local params = ...

local headers = params.headers
local libs = params.libs
local platform = params.platform
local builddir = params.builddir

-- let's curl it up a notch
headers.libcurl = {}
local path = "glorp/libs/curl-7.20.0"

local files = {}

ursa.token.rule{"curl_files", nil, function () return ursa.system{("cd %s && (((find . -type f | grep -v CMake | grep -v docs | grep -v tests) && (find . -type f | grep Makefile)) | sed s*\\\\./** | sort | uniq)"):format(path)} end}
for item in ursa.token{"curl_files"}:gmatch("([^\n]+)") do
  if item == "Makefile" or item == "include/curl/curlbuild.h" then
  elseif item == "Makefile.in" then
    table.insert(files, ursa.rule{builddir .. "lib_build/curl/" .. item, path .. "/" .. item, ursa.util.system_template{('sed -e "s@SUBDIRS = lib src@SUBDIRS = lib@" $SOURCE > $TARGET'):format(ursa.token{"CC"})}})
  elseif item == "include/curl/curlbuild.h.in" then
    table.insert(files, ursa.rule{builddir .. "lib_build/curl/" .. item, path .. "/" .. item, function ()
      print("stupid reparse of curlbuild.h.in")
      local fin = io.open(path .. "/" .. item, "r")
      local fout = io.open(builddir .. "lib_build/curl/" .. item, "w")
      while true do
        local lin = fin:read("*line")
        if not lin then break end
        fout:write(lin .. "\n")
        if lin == "#define __CURL_CURLBUILD_H" then
          fout:write("#define CURL_STATICLIB" .. "\n")
        end
      end
      fin:close()
      fout:close()
    end})
  elseif item == "configure"  and platform == "cygwin" then
    table.insert(files, ursa.rule{builddir .. "lib_build/curl/" .. item, path .. "/" .. item, ursa.util.system_template{([[sed
      -e "s/for sel_arg1 in/for sel_arg1 in 'int'/"
      -e "s/for sel_arg234 in/for sel_arg234 in 'fd_set *'/"
      -e "s/for sel_arg5 in/for sel_arg5 in 'const struct timeval *'/"
      -e "s/for sel_retv in/for sel_retv in 'int'/"
      
      -e "s/for recv_arg1 in/for recv_arg1 in 'SOCKET'/"
      -e "s/for recv_arg2 in/for recv_arg2 in 'char *'/"
      -e "s/for recv_arg3 in/for recv_arg3 in 'int'/"
      -e "s/for recv_arg4 in/for recv_arg4 in 'int'/"
      -e "s/for recv_retv in/for recv_retv in 'int'/"
      
      -e "s/for recvfrom_arg1 in/for recvfrom_arg1 in 'SOCKET'/"
      -e "s/for recvfrom_arg2 in/for recvfrom_arg2 in 'char *'/"
      -e "s/for recvfrom_arg3 in/for recvfrom_arg3 in 'int'/"
      -e "s/for recvfrom_arg4 in/for recvfrom_arg4 in 'int'/"
      -e "s/for recvfrom_arg5 in/for recvfrom_arg5 in 'struct sockaddr *'/"
      -e "s/for recvfrom_arg6 in/for recvfrom_arg6 in 'int *'/"
      -e "s/for recvfrom_retv in/for recvfrom_retv in 'int'/"
      
      -e "s/for send_arg1 in/for send_arg1 in 'SOCKET'/"
      -e "s/for send_arg2 in/for send_arg2 in 'const char *'/"
      -e "s/for send_arg3 in/for send_arg3 in 'int'/"
      -e "s/for send_arg4 in/for send_arg4 in 'int'/"
      -e "s/for send_retv in/for send_retv in 'int'/"
      
      $SOURCE > $TARGET && chmod +x $TARGET]]):format(ursa.token{"CC"}):gsub("\n", " ")}})
  else
    table.insert(files, ursa.rule{builddir .. "lib_build/curl/" .. item, path .. "/" .. item, ursa.util.copy{}})
  end
end

local makefile = ursa.rule{{builddir .. "lib_build/curl/Makefile", builddir .. "lib_build/curl/include/curl/curlbuild.h"}, files, ursa.util.system_template{('cd %slib_build/curl && CC="#CC" CFLAGS="#CCFLAGS" LDFLAGS="#LDFLAGS" ./configure --disable-shared --disable-dependency-tracking --disable-ftp --disable-file --disable-ldap --disable-ldaps --disable-rtsp --disable-proxy --disable-dict --disable-telnet --disable-tftp --disable-pop3 --disable-imap --disable-smtp --disable-manual --disable-ipv6 --disable-crypto-auth --disable-cookies'):format(builddir)}}

local lib = ursa.rule{builddir .. "lib_build/curl/lib/.libs/libcurl.a", makefile, ('cd %slib_build/curl && make'):format(builddir)}  -- can't parallelize this, libcurl is a giant butt

libs.libcurl = ursa.rule{builddir .. "lib_release/lib/libcurl.a", lib, ursa.util.copy{}}
table.insert(headers.libcurl, ursa.rule{builddir .. "lib_release/include/curl/curl.h", builddir .. "lib_build/curl/include/curl/curl.h", ursa.util.copy{}})
table.insert(headers.libcurl, ursa.rule{builddir .. "lib_release/include/curl/curlver.h", builddir .. "lib_build/curl/include/curl/curlver.h", ursa.util.copy{}})
table.insert(headers.libcurl, ursa.rule{builddir .. "lib_release/include/curl/curlbuild.h", builddir .. "lib_build/curl/include/curl/curlbuild.h", ursa.util.copy{}})
table.insert(headers.libcurl, ursa.rule{builddir .. "lib_release/include/curl/curlrules.h", builddir .. "lib_build/curl/include/curl/curlrules.h", ursa.util.copy{}})
table.insert(headers.libcurl, ursa.rule{builddir .. "lib_release/include/curl/easy.h", builddir .. "lib_build/curl/include/curl/easy.h", ursa.util.copy{}})
table.insert(headers.libcurl, ursa.rule{builddir .. "lib_release/include/curl/multi.h", builddir .. "lib_build/curl/include/curl/multi.h", ursa.util.copy{}})