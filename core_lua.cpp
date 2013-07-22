
#include "args.h"
#include "core.h"
#include "debug.h"
#include "os.h"
#include "pak.h"
#include "perfbar.h"
#include "util.h"
#include "version.h"

#include "lgl.h"

#include <lua.hpp>

#include <boost/random.hpp>

#include <ctime>

#undef printf
#include "frame/frame.h"
#include "frame/texture.h"
#include "frame/mask.h"
#include "frame/text.h"
#include "frame/lua.h"
#include "frame/stream.h"
#define printf FAILURE

using namespace std;

DEFINE_bool(debug, false, "Debug flag")

namespace Glorp {

  class FrameLogger : public Frame::Configuration::Logger {
  public:
    virtual void LogDebug(const std::string &log) {
      dprintf("Frame debug: %s", log.c_str());
    }
    virtual void LogError(const std::string &log) {
      dprintf("Frame error: %s", log.c_str());
    }
  };
  static FrameLogger frame_logger;

  class FramePerformance : public Frame::Configuration::Performance {
  public:
    virtual void *Push(float r, float g, float b) {
      return new PerfStack(r, g, b);
    }
    virtual void Pop(void *handle) {
      delete (PerfStack*)handle;
    }
  };
  static FramePerformance frame_performance;

  class FramePath : public Frame::Configuration::PathFromId {
  public:
    virtual std::string Process(Frame::Environment *env, const std::string &id) {
      // We also append an extension to it if applicable
      std::string prefixed = "data/" + id;
      if (PakHas(prefixed + ".png")) return prefixed + ".png";
      if (PakHas(prefixed + ".jpg")) return prefixed + ".jpg";
      if (PakHas(prefixed + ".ttf")) return prefixed + ".ttf";
      return "data/" + id;
    }
  };
  static FramePath frame_path;
  
  class FrameStream : public Frame::Configuration::StreamFromId {
  public:
    virtual Frame::Stream *Create(Frame::Environment *env, const std::string &id) {
      std::string path = env->GetConfiguration().pathFromId->Process(env, id);
      if (!path.empty()) {
        std::vector<unsigned char> data;
        PakRead(path, &data);
        return Frame::StreamBuffer::Create(data);
      }

      return 0;
    }
  };
  static FrameStream frame_stream;

  // get our own rng. why? because it turns out that lua does weird things with RNGs across coroutines
  boost::lagged_fibonacci9689 rngstate(time(NULL));
  static int math_random (lua_State *L) {
    /* the `%' avoids the (rare) case of r==1, and is needed also because on
       some systems (SunOS!) `rand()' may return a value larger than RAND_MAX */
    lua_Number r = rngstate();
    switch (lua_gettop(L)) {  /* check number of arguments */
      case 0: {  /* no arguments */
        lua_pushnumber(L, r);  /* Number between 0 and 1 */
        break;
      }
      case 1: {  /* only upper limit */
        int u = luaL_checkint(L, 1);
        luaL_argcheck(L, 1<=u, 1, "interval is empty");
        lua_pushnumber(L, floor(r*u)+1);  /* int between 1 and `u' */
        break;
      }
      case 2: {  /* lower and upper limits */
        int l = luaL_checkint(L, 1);
        int u = luaL_checkint(L, 2);
        luaL_argcheck(L, l<=u, 2, "interval is empty");
        lua_pushnumber(L, floor(r*(u-l+1))+l);  /* int between `l' and `u' */
        break;
      }
      default: return luaL_error(L, "wrong number of arguments");
    }
    return 1;
  }
  static int math_randomseed (lua_State *L) {
    rngstate = boost::lagged_fibonacci9689(luaL_checkint(L, 1));
    return 0;
  }

  static int debug_print(lua_State *L) {
    string acul = "";
    int n = lua_gettop(L);  /* number of arguments */
    int i;
    lua_getglobal(L, "tostring");
    for (i=1; i<=n; i++) {
      const char *s;
      lua_pushvalue(L, -1);  /* function to be called */
      lua_pushvalue(L, i);   /* value to print */
      lua_call(L, 1, 1);
      s = lua_tostring(L, -1);  /* get result */
      if (s == NULL)
        return luaL_error(L, LUA_QL("tostring") " must return a string to "
                             LUA_QL("print"));
      if (i>1) acul += "\t";
      acul += s;
      lua_pop(L, 1);  /* pop result */
    }
    dprintf("%s", acul.c_str());
    return 0;
  }
  
  struct ReaderData {
    ReaderData() : finished(false) { }
    
    std::vector<unsigned char> dat;
    bool finished;
  };
  
  static const char *package_reader(lua_State *L, void *ud, size_t *size)
  {
    ReaderData *rdat = (ReaderData *)ud;
    if (rdat->finished) {
      return 0;
    } else {
      rdat->finished = true;
      *size = rdat->dat.size();
      return (const char *)&rdat->dat[0];
    }
  }
  
  static int package_loadfile(lua_State *L) {
    const char *fname = luaL_checkstring(L, 1);
    
    if (!PakHas(fname)) {
      lua_pushnil(L);
      lua_pushfstring(L, "cannot open %s: No such file or directory", fname);
      return 2;
    }
    
    ReaderData rdat;
    PakRead(fname, &rdat.dat);
    
    if (lua_load(L, package_reader, &rdat, Format("@%s", fname).c_str())) {
      // error
      lua_pushnil(L);
      lua_insert(L, -2);
      return 2;
    } else {
      // no error
      return 1;
    }
  }
  
  void Core::l_init() {
    CHECK(!m_L);
    if (m_L)
      l_shutdown();

    dprintf("Environment initialize!");

    Frame::Configuration config;
    config.logger = &frame_logger;
    config.performance = &frame_performance;
    config.pathFromId = &frame_path;
    config.streamFromId = &frame_stream;
    config.fontDefaultId = Version::gameFontDefault;

    m_env = new Frame::Environment(config);
    m_env->ResizeRoot(Version::gameXres, Version::gameYres);
    
    m_luaCrashed = false;

    m_L = lua_open();   /* opens Lua */
    lua_State *L = m_L;
    luaL_openlibs(L);

    luaopen_lgl(L);

    CHECK(lua_gettop(L) == 1);

    lua_pop(L, 1);

    CHECK(lua_gettop(L) == 0);

    // replace the RNG
    lua_getglobal(L, "math");
    lua_pushcfunction(L, math_random);
    lua_setfield(L, -2, "random");
    lua_pushcfunction(L, math_randomseed);
    lua_setfield(L, -2, "randomseed");
    lua_pop(L, 1);

    // replace print
    lua_pushcfunction(L, debug_print);
    lua_setglobal(L, "print");
    
    // replace loadfile
    lua_pushcfunction(L, package_loadfile);
    lua_setglobal(L, "loadfile");

    // push registry
    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, "Glorp");

    CHECK(lua_gettop(L) == 0);

    m_env->LuaRegister(L);

    CHECK(lua_gettop(L) == 0);

    // the bulk of the lua init
    {
      // boot up our bootstrap file
      lua_getglobal(L, "loadfile");
      lua_pushstring(L, "data/glorp/init_bootstrap.lua");
      if (lua_pcall(L, 1, 2, 0)) {
        dprintf("Init crashed!");
        CHECK(0);
        m_luaCrashed = true;
        lua_pop(L, 2);
        return;
      }
      
      if (lua_isnil(L, -2)) {
        lua_pop(L, 2);
        
        // try again with a different path
        lua_getglobal(L, "loadfile");
        lua_pushstring(L, "glorp/init_bootstrap.lua");
        if (lua_pcall(L, 1, 2, 0)) {  // we want to include the error message this time to make debugging easier
          dprintf("Init crashed!");
          CHECK(0);
          m_luaCrashed = true;
          lua_pop(L, 2);
          return;
        }
      }
      
      if (lua_isnil(L, -2)) {
        // welp, crashed - dump error message and abort
        CHECK(0, "%s", lua_tostring(L, -1));
        m_luaCrashed = true;
        lua_pop(L, 2);
        return;
      }
      
      // toss the error message
      lua_pop(L, 1);

      // call our bootstrap function
      if (lua_pcall(L, 0, 1, 0)) {
        dprintf("Init crashed!");
        CHECK(0);
        m_luaCrashed = true;
        lua_pop(L, 1);
        return;
      }

      // we should now have our bootstrap package
      CHECK(lua_gettop(L) == 1);

      lua_getfield(L, 1, "Wrap");
      m_func_wrap = l_register(L);

      // create events
      m_event_system_update_begin = l_registerEvent(L, "System.Update.Begin");
      m_event_system_update_end = l_registerEvent(L, "System.Update.End");

      m_event_system_mouse = l_registerEvent(L, "System.Mouse");
      m_event_system_key_down = l_registerEvent(L, "System.Key.Down");
      m_event_system_key_up = l_registerEvent(L, "System.Key.Up");
      m_event_system_key_type = l_registerEvent(L, "System.Key.Type");
      m_event_system_key_repeat = l_registerEvent(L, "System.Key.Repeat");

      // create functions
      l_registerInspect(L, "System.Time.Real", l_system_time_real);

      // kick off the load of the actual game
      lua_getfield(L, 1, "Wrap");
      lua_getfield(L, 1, "InitComplete");
      if (lua_pcall(L, 1, 0, 0))
      {
        dprintf("Init crashed!");
        CHECK(0);
        m_luaCrashed = true;
        lua_pop(L, 1);
        return;
      }

      lua_pop(L, 1);
    }

    CHECK(lua_gettop(L) == 0);
  }
  void Core::l_shutdown() {
    delete m_env;
    m_env = 0;

    lua_close(m_L);
    m_L = 0;

    dprintf("Environment shutdown.");
    dprintf("==============================================");
  }

  int Core::l_register(lua_State *L) {
    lua_pushliteral(L, "Glorp");
    lua_gettable(L, LUA_REGISTRYINDEX);

    lua_pushvalue(L, -2);
    int ref = luaL_ref(L, -2);
    lua_pop(L, 2);

    return ref;
  }

  void Core::l_retrieve(lua_State *L, int index) {
    assert(index != LUA_NOREF);

    lua_pushliteral(L, "Glorp");
    lua_gettable(L, LUA_REGISTRYINDEX);

    lua_rawgeti(L, -1, index);
    assert(!lua_isnil(L, -1));
    lua_replace(L, -2);
  }

  void Core::l_registerInspect(lua_State *L, const char *name, int (*func)(lua_State *)) {
    l_retrieve(L, m_func_wrap);
    lua_getfield(L, -2, "InsertItem");
    lua_pushvalue(L, LUA_GLOBALSINDEX);
    lua_pushstring(L, (std::string("Inspect.") + name).c_str());
    lua_pushcfunction(L, func);
    if (lua_pcall(L, 4, 0, 0)) {
      dprintf("register pcall error");
      m_luaCrashed = true;
      CHECK(0);
      lua_pop(L, 1);
    }
  }

  /*static*/ int Core::l_system_time_real(lua_State *L) {
    Frame::luaF_checkparams(L, 0);

    lua_pushnumber(L, timeMicro() / 1000000.);

    return 1;
  }

  int Core::l_registerEvent(lua_State *L, const char *event) {
    l_retrieve(L, m_func_wrap);
    lua_getfield(L, -2, "CreateEvent");
    lua_pushvalue(L, LUA_GLOBALSINDEX);
    lua_pushstring(L, event);
    if (lua_pcall(L, 3, 1, 0)) {
      dprintf("register pcall error");
      m_luaCrashed = true;
      CHECK(0);
      lua_pop(L, 1);
      return LUA_NOREF;
    }
    return l_register(L);
  }

  void Core::l_callEvent(lua_State *L, int event, int params) {
    CHECK(m_L && !m_luaCrashed);

    // insert these two before the params
    l_retrieve(L, m_func_wrap);
    lua_insert(L, -(params + 1));
    l_retrieve(L, event);
    lua_insert(L, -(params + 1));

    if (lua_pcall(L, 1 + params, 0, 0)) {
      dprintf("call event error");
      m_luaCrashed = true;
      if (!FLAGS_debug) {
        CHECK(0);
      }
      lua_pop(L, 1);
    }
  }
}
