
#include "args.h"
#include "core.h"
#include "debug.h"
#include "os.h"
#include "perfbar.h"
#include "version.h"

#include "lal.h"
#include "lgl.h"

#include <lua.hpp>

#include <boost/random.hpp>

#include <ctime>

#undef printf
#include <frames/frame.h>
#include <frames/texture.h>
#include <frames/mask.h>
#include <frames/text.h>
#define printf FAILURE

using namespace std;

namespace Glorp {

  class FramesLogger : public Frames::Configuration::Logger {
  public:
    virtual void LogDebug(const std::string &log) {
      dprintf("Frames debug: %s", log.c_str());
    }
    virtual void LogError(const std::string &log) {
      dprintf("Frames error: %s", log.c_str());
    }
  };
  static FramesLogger frames_logger;

  class FramesPerformance : public Frames::Configuration::Performance {
  public:
    virtual void *Push(float r, float g, float b) {
      return new PerfStack(r, g, b);
    }
    virtual void Pop(void *handle) {
      delete (PerfStack*)handle;
    }
  };
  static FramesPerformance frames_performance;

  class FramesPath : public Frames::Configuration::PathFromId {
  public:
    virtual std::string Process(Frames::Environment *env, const std::string &id) {
      return "data/" + id;
    }
  };
  static FramesPath frames_path;

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

  void PrintWoopWoop() {
    dprintf("Woop woop!");
  }

  void Core::l_init() {
    CHECK(!m_L);
    if (m_L)
      l_shutdown();

    dprintf("Environment initialize!");

    Frames::Configuration config;
    config.logger = &frames_logger;
    config.performance = &frames_performance;
    config.pathFromId = &frames_path;
    config.fontDefaultId = Version::gameFontDefault;

    m_env = new Frames::Environment(config);
    m_env->ResizeRoot(Version::gameXres, Version::gameYres);
    
    m_luaCrashed = false;

    m_L = lua_open();   /* opens Lua */
    lua_State *L = m_L;
    luaL_openlibs(L);

    luaopen_lgl(L);
    luaopen_lal(L);

    CHECK(lua_gettop(L) == 2);

    lua_pop(L, 2);

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

    // push registry
    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, "Glorp");

    CHECK(lua_gettop(L) == 0);

    m_env->LuaRegister(L);
    m_env->LuaRegisterFramesBuiltin(L);

    CHECK(lua_gettop(L) == 0);

    // the bulk of the lua init
    {
      int error = luaL_loadfile(L, "data/glorp/init_bootstrap.lua");
      if(error) {
        lua_pop(L, 1);
        error = luaL_loadfile(L, "glorp/init_bootstrap.lua");
      }
      if(error) {
        CHECK(0, "%s", lua_tostring(L, -1));
        m_luaCrashed = true;
        lua_pop(L, 1);
        return;
      }

      if (lua_pcall(L, 0, 1, 0))
      {
        dprintf("Init crashed!");
        CHECK(0);
        m_luaCrashed = true;
        lua_pop(L, 1);
        return;
      }

      CHECK(lua_gettop(L) == 1);

      lua_getfield(L, 1, "Wrap");
      m_func_wrap = l_register(L);

      m_event_system_update_begin = l_registerEvent(L, "System.Update.Begin");
      m_event_system_update_end = l_registerEvent(L, "System.Update.End");

      m_event_system_mouse = l_registerEvent(L, "System.Mouse");
      m_event_system_key_down = l_registerEvent(L, "System.Key.Down");
      m_event_system_key_up = l_registerEvent(L, "System.Key.Up");
      m_event_system_key_type = l_registerEvent(L, "System.Key.Type");
      m_event_system_key_repeat = l_registerEvent(L, "System.Key.Repeat");

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
      CHECK(0);
      lua_pop(L, 1);
    }
  }
}
