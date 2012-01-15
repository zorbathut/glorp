
#include "args.h"
#include "core.h"
#include "debug.h"
#include "os.h"
#include "version.h"

#include "GLee.h"

#include "lal.h"
#include "lgl.h"

#include <lua.hpp>

#include <boost/random.hpp>

#include <ctime>

using namespace std;

namespace Glorp {

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

  void Core::lua_init() {
    CHECK(!L);
    if (L)
      lua_shutdown();

    m_luaCrashed = false;

    L = lua_open();   /* opens Lua */
    luaL_openlibs(L);

    luaopen_lgl(L);
    luaopen_lal(L);

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

    // the bulk of the lua init
    {
      int error = luaL_loadfile(L, "data/glorp/init_bootstrap.lua");
      if(error) {
        error = luaL_loadfile(L, "glorp/init_bootstrap.lua");
      }
      if(error) {
        CHECK(0, "%s", lua_tostring(L, -1));
        m_luaCrashed = true;
        lua_pop(L, 1);
        return;
      }

      if (lua_pcall(L, 0, 0, 0))
      {
        dprintf("Init crashed!");
        m_luaCrashed = true;
        lua_pop(L, 1);
      }
    }
  }
  void Core::lua_shutdown() {
    lua_close(L);
    L = 0;
  }
}
