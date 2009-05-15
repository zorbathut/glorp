
import os
import commands
import SCons.dblite

from SCons.Environment import Environment
from SCons.Util import Split

import SCons.Node.FS

import sys

class SconToken:  # man, don't even ask
  def __init__(self, toki):
    self.toki = str(toki)
    
  def convert_to_sconsign(self):
    pass
  def convert_from_sconsign(self, dir, key):
    pass

def Conf():

  # Set up our environment
  env = Environment(LINKFLAGS = Split("-g -O2 -Wl,--as-needed"), CXXFLAGS=Split("-Wall -Wno-sign-compare -Wno-uninitialized -g -O2"), CPPDEFINES=["DPRINTF_MARKUP"], CXX="nice glorp/ewrap $TARGET g++")
  
  categories = Split("GAME")
  flagtypes = Split("CCFLAGS CPPFLAGS CXXFLAGS LINKFLAGS LIBS CPPPATH LIBPATH CPPDEFINES")
  
  for flag in flagtypes:
    env.Append(**{flag : []})
    for item in categories:
      env[flag + "_" + item] = []
  
  if sys.platform == "cygwin": # Cygwin
    platform="win"
  elif sys.platform == "linux2":
    platform="linux"
  else:
    print "Platform is unrecognized platform " + sys.platform
    env.Exit(1)

  # If we're cleaning, we don't need all this.
  if not env.GetOption('clean'):
    
    # Here's our custom tests
    def CheckFile(context, paths, file):
      context.Message("Checking for %s ... " % file)
      db = SCons.Node.FS.get_default_fs().Dir(".").sconsign().entries
      ki = str(("CheckFile", paths, file))
      if ki in db:
        context.sconf.cached = 1
        rv = db[ki].toki
      else:
        context.sconf.cached = 0
        rv = 0
        for path in paths:
          testpath = path + "/" + file;
          if os.path.exists(testpath):
            rv = '"%s"' % testpath
            db[ki] = SconToken(rv)
            break
      context.Result(rv)
      return rv

    def Execute(context, command):
      context.Message("Caching return value of %s ... " % command)
      db = SCons.Node.FS.get_default_fs().Dir(".").sconsign().entries
      ki = str(("Execute", command))
      if ki in db:
        context.sconf.cached = 1
        rv = db[ki].toki
      else:
        context.sconf.cached = 0
        (status, rv) = commands.getstatusoutput(command)
        if status != 0:
          rv = None
        db[ki] = SconToken(rv)
      context.Result(rv)
      return rv
      

    # Now let's actually configure it
    conf = env.Configure(custom_tests = {'CheckFile' : CheckFile, 'Execute' : Execute})

    curldepend=""

    if platform == "win": # Cygwin
      env.Append(CCFLAGS=Split("-mno-cygwin"), CPPFLAGS=Split("-mno-cygwin"), CXXFLAGS=Split("-mno-cygwin"), LINKFLAGS=Split("-mwindows -mno-cygwin"))
      env.Append(CPPPATH=["glop/Glop/cygwin/include", "lua/include", "/usr/mingw/local/include"], LIBPATH=["glop/Glop/cygwin/lib", "/lib/mingw", "lua/lib", "/usr/mingw/local/lib"])
      env.Append(CPPPATH_GAME = "glop/build-dbg-Glop")
      env.Append(CPPDEFINES = "WIN32")
      
      if not conf.CheckLibWithHeader("opengl32", "GL/gl.h", "c++", "glLoadIdentity();", autoadd=0):
        env.Exit(1)
      env.Append(LIBS_GAME="opengl32")
      
      if not conf.CheckLibWithHeader("freetype", "ft2build.h", "c++", "main();", autoadd=0):
        env.Exit(1)
      env.Append(LIBS_GAME="freetype")
      
      #if not conf.CheckLibWithHeader("jpeg", "jpeglib/jpeglib.h", "c++", "jpeg_std_error();", autoadd=0):
        #env.Exit(1)
      env.Append(LIBS_GAME="jpeg")
      
      #if not conf.CheckLibWithHeader("dxguid", "dinput.h", "c++", "GUID_SysKeyboard;", autoadd=0):
        #env.Exit(1)
      env.Append(LIBS_GAME=["dinput", "dxguid", "mingw32", "pthread"])
      
      env.Append(LIBS_GAME=["winmm"])
      
      env.Append(LIBS_GAME=['kernel32','user32','gdi32','winspool','comdlg32','advapi32','shell32','ole32','oleaut32','uuid','odbc32','odbccp32', 'opengl32', 'freetype', 'jpeg', 'glu32', 'dinput', 'dxguid', 'fmodex', 'winmm'])
      
      # Boost flags
      boostlibs=[]
      boostpath=["/usr/mingw/local/include/boost-1_38_0"]
      
      env.Append(ENV = {"PATH" : os.environ['PATH'], "TEMP" : os.environ['TEMP']})  # this is not really ideal
      
      # Set up our environment defaults
      if False:
        env.Append(CPPPATH = Split("/usr/mingw/local/include"), LIBPATH = Split("/usr/mingw/local/lib"), CCFLAGS=Split("-mno-cygwin"), CPPFLAGS=Split("-mno-cygwin"), CXXFLAGS=Split("-mno-cygwin"), LINKFLAGS=Split("-mwindows -mno-cygwin"))
        

        
        # VECTOR_PARANOIA
        env.Append(CPPDEFINES=["VECTOR_PARANOIA"])
        
        if not conf.CheckCXXHeader('windows.h', '<>'):
          env.Exit(1)
        
        if not conf.CheckLibWithHeader("ws2_32", "windows.h", "c++", "WSACleanup();", autoadd=0):
          env.Exit(1)
        env.Append(LIBS_GAME="ws2_32")
        env.Append(LIBS_REPORTER="ws2_32")
        

        
        if not conf.CheckLib("mingw32", autoadd=0):
          env.Exit(1)
        env.Append(LIBS="mingw32")

        curldepend="ws2_32"
        
        # Check for makensis
        installer = conf.CheckFile(["/cygdrive/c/Program Files (x86)/NSIS"], "makensis")
        if not installer:
          env.Exit(1)
          
        defaultdata = ""
      
    elif platform == "linux":
      env.Append(CPPDEFINES = Split("NO_WINDOWS"), CPPPATH = Split("/usr/local/include/boost-1_34_1"))
      
      boostlibs=["boost_regex", "boost_filesystem"]
      boostpath=["/usr/local/include/boost-1_34_1"]
      
      if not conf.CheckLibWithHeader("GL", "GL/gl.h", "c++", "glBegin(0);", autoadd=0):
        env.Exit(1)
      env.Append(LIBS_GAME="GL")

      pkgconfig = conf.CheckFile(["/usr/bin"], "pkg-config")
      if not pkgconfig:
        env.Exit(1)
      pkgstring = conf.Execute("%s --libs --cflags gtk+-2.0" % pkgconfig)
      if not pkgstring:
        env.Exit(1)
      env.MergeFlags(dict([(k + "_GAME", v) for k, v in env.ParseFlags(pkgstring).items()]))
      env.MergeFlags(dict([(k + "_REPORTER", v) for k, v in env.ParseFlags(pkgstring).items()]))
 
      env.Append(ENV = os.environ)
#      env.Append(ENV = {"DISPLAY" : os.environ['DISPLAY']})  # welp

      installer = None
      
      defaultdata = "/usr/share/d-net/"
      
      convert = conf.CheckFile(["/usr/bin"], "convert")
      if not convert:
        env.Exit(1)
    else:
      print "Platform is unrecognized platform " + sys.platform
      env.Exit(1)

    # Boost
    env.Append(LIBS=boostlibs, CPPPATH=boostpath)
    if not conf.CheckCXXHeader('boost/noncopyable.hpp', '<>'):
      env.Exit(1)
    for lib in boostlibs:
      if not conf.CheckLib(lib, autoadd=0):
        env.Exit(1)

    if not conf.CheckLibWithHeader("lua", "lua.hpp", "c++", "lua_open();", autoadd=0):
      env.Exit(1)
    env.Append(LIBS="lua")
    
    env.Prepend(LIBS="luabind")
    if not conf.CheckLibWithHeader(None, "luabind/luabind.hpp", "c++", "luabind::open(NULL);", autoadd=0):
      env.Exit(1)
    
    
    if False:
      # libm
      if not conf.CheckLib("m", autoadd=0):
        env.Exit(1)
      env.Append(LIBS="m")

      # zlib
      if not conf.CheckLib("z", "compress", autoadd=0):
        env.Exit(1)
      env.Append(LIBS_GAME="z")
      env.Append(LIBS_EDITOR="z")
      env.Append(LIBS_REPORTER="z")
      env.Append(LIBS_CONSOLE_MERGER="z")
      env.Append(LIBS_CONSOLE_ODS2CSV="z")

      # libpng
      if not conf.CheckLib("png", "png_create_read_struct", autoadd=0):
        env.Exit(1)
      env.Append(LIBS_EDITOR="png")

      # curl
      curlpath = conf.CheckFile(["/usr/mingw/local/bin", "/usr/local/bin", "/usr/bin"], "curl-config")
      if not curlpath:
        env.Exit(1)
      env.MergeFlags(dict([(k + "_REPORTER", v) for k, v in env.ParseFlags(conf.Execute(curlpath + " --cflags --static-libs")).items()]))
      env.Append(CPPDEFINES_REPORTER="CURL_STATICLIB") # sigh

      # xerces
      if not conf.CheckLib("xerces-c", autoadd=0):
        env.Exit(1)
      env.Append(LIBS_CONSOLE_ODS2CSV="xerces-c")

      # Check for libogg
      if not conf.CheckLib("ogg", "ogg_stream_init", autoadd=0):
        env.Exit(1)
      env.Append(LIBS_GAME="ogg")
      
      # Check for libvorbis
      if not conf.CheckLib("vorbis", "vorbis_info_init", autoadd=0):
        env.Exit(1)
      env.Append(LIBS_GAME="vorbis")
      
      # Check for libvorbisfile
      if not conf.CheckLibWithHeader("vorbisfile", "vorbis/vorbisfile.h", "c++", 'ov_fopen("hi", NULL);', autoadd=0):
        env.Exit(1)
      env.Append(LIBS_GAME="vorbisfile")

      # Check for wx
      wxpath = conf.CheckFile(["/usr/mingw/local/bin", "/usr/local/bin", "/usr/bin"], "wx-config")
      if not wxpath:
        env.Exit(1)
      env.MergeFlags(dict([(k + "_EDITOR", v) for k, v in env.ParseFlags(conf.Execute(wxpath + " --libs --cxxflags --gl-libs")).items()]))

      # Check for oggenc
      oggpath = conf.CheckFile(["/cygdrive/c/windows/util", "/usr/bin"], "oggenc")
      if not oggpath:
        env.Exit(1)

    env = conf.Finish()
    
  else:
    
    oggpath = ""
    installer = ""
    defaultdata = ""
  
  return env, categories, flagtypes, platform
