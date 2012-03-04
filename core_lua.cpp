
#include "args.h"
#include "core.h"
#include "debug.h"
#include "os.h"
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

    Frames::Configuration config;
    config.logger = &frames_logger;

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
      }

      CHECK(lua_gettop(L) == 1);

      lua_getfield(L, 1, "Wrap");
      m_func_wrap = l_register(L);

      m_event_system_update_begin = l_registerEvent(L, "System.Update.Begin");
      m_event_system_update_end = l_registerEvent(L, "System.Update.End");

      m_event_system_mouse = l_registerEvent(L, "System.Mouse");
      m_event_system_key = l_registerEvent(L, "System.Key");

      // kick off the load of the actual game
      lua_getfield(L, 1, "Wrap");
      lua_getfield(L, 1, "InitComplete");
      if (lua_pcall(L, 1, 0, 0))
      {
        dprintf("Init crashed!");
        CHECK(0);
        m_luaCrashed = true;
        lua_pop(L, 1);
      }

      lua_pop(L, 1);
    }

    CHECK(lua_gettop(L) == 0);

    Frames::Frame *winframe = Frames::Frame::CreateTagged(m_env->GetRoot());

    winframe->SetPoint(Frames::X, 0, m_env->GetRoot(), 0, 40);
    winframe->SetPoint(Frames::Y, 0, m_env->GetRoot(), 0, 40);
    winframe->SetWidth(300);
    winframe->SetHeight(600);

    winframe->SetBackground(0.3, 0.3, 0.3);

    Frames::Frame *title = Frames::Frame::CreateTagged(winframe);

    title->SetPoint(Frames::X, 0, winframe, 0, 10);
    title->SetPoint(Frames::X, 1, winframe, 1, -10);
    title->SetPoint(Frames::Y, 0, winframe, 0, 10);
    title->SetHeight(20);

    title->SetBackground(1, 1, 0, 0.5);

    Frames::Frame *ok = Frames::Frame::CreateTagged(winframe);

    ok->SetPoint(Frames::X, 1, winframe, 1, -10);
    ok->SetPoint(Frames::Y, 1, winframe, 1, -10);
    ok->SetHeight(20);
    ok->SetWidth(80);

    ok->SetBackground(0, 1, 0, 0.5);

    Frames::Frame *aframe = Frames::Frame::CreateTagged(m_env->GetRoot());
    Frames::Frame *bframe = Frames::Frame::CreateTagged(m_env->GetRoot());
    Frames::Frame *cframe = Frames::Frame::CreateTagged(m_env->GetRoot());

    aframe->SetPoint(Frames::X, 0, bframe, 0, 0);
    bframe->SetPoint(Frames::X, 0, cframe, 0, 0);
    cframe->SetPoint(Frames::X, 0, aframe, 0, 0);

    Frames::Frame *gcframe = Frames::Frame::CreateTagged(m_env->GetRoot());
    gcframe->Obliterate();

    aframe->EventMoveAttach(PrintWoopWoop);


    Frames::Mask *mask = Frames::Mask::CreateTagged(m_env->GetRoot());
    mask->SetPoint(Frames::X, 0, m_env->GetRoot(), 0.5, 0);
    mask->SetPoint(Frames::Y, 0, m_env->GetRoot(), 0.5, 0);
    mask->SetPoint(Frames::X, 1, m_env->GetRoot(), 1, 0);
    mask->SetPoint(Frames::Y, 1, m_env->GetRoot(), 1, 0);

    Frames::Texture *tex = Frames::Texture::CreateTagged(mask);
    tex->SetPoint(Frames::X, 0.5, m_env->GetRoot(), 0.5, 0);
    tex->SetPoint(Frames::Y, 0.5, m_env->GetRoot(), 0.5, 0);
    tex->SetBackground(1, 1, 1, 0.1);
    tex->SetTexture("awesome_med.png");

    Frames::Texture *tex2 = Frames::Texture::CreateTagged(mask);
    tex2->SetPoint(Frames::X, 0.5, m_env->GetRoot(), 0.8, 0);
    tex2->SetPoint(Frames::Y, 0.5, m_env->GetRoot(), 0.8, 0);
    tex2->SetBackground(1, 1, 1, 0.1);
    tex2->SetTexture("mind-in-pictures-january-12_1_thumb.jpg");
    
    Frames::Text *texu = Frames::Text::CreateTagged(m_env->GetRoot());
    texu->SetPoint(Frames::X, 0, m_env->GetRoot(), 0, 120);
    texu->SetPoint(Frames::Y, 0, m_env->GetRoot(), 0, 120);
    texu->SetBackground(1, 0, 0, 0.1);
    texu->SetText("AVAVAVAVAVAVAVAV");
    texu->SetSize(30);
    texu->SetLayer(3);

    Frames::Text *texix = Frames::Text::CreateTagged(m_env->GetRoot());
    texix->SetPoint(Frames::X, 0, m_env->GetRoot(), 0, 400);
    texix->SetPoint(Frames::Y, 0, m_env->GetRoot(), 0, 80);
    texix->SetBackground(0, 1, 0, 0.1);
    texix->SetText("lend me your arms,\nfast as thunderbolts,\nfor a pillow on my journey.");
    texix->SetSize(50);
    texix->SetLayer(3);

    texix->SetWidth(texix->GetWidth() - 50);
    texix->SetHeight(texix->GetHeight() - 20);

    Frames::Text *texite = Frames::Text::CreateTagged(m_env->GetRoot());
    texite->SetPoint(Frames::X, 0, m_env->GetRoot(), 0, 100);
    texite->SetPoint(Frames::Y, 0, m_env->GetRoot(), 0, 250);
    texite->SetWidth(600);
    texite->SetBackground(0, 0, 1, 0.1);
    texite->SetColor(1, 0.8, 0.8, 0.8);
    texite->SetText("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Duis urna libero, elementum id commodo et, mollis et neque. Sed suscipit ornare purus, sed pellentesque felis luctus eu. Maecenas gravida, odio quis fermentum pretium, libero metus lacinia justo, a fringilla neque nisl vel lacus. Praesent elementum mauris et ligula dictum porttitor. Pellentesque a risus quam. Aliquam tincidunt interdum viverra. Nam quis nisi neque. Nam non risus tellus, ac ullamcorper eros. Mauris vestibulum odio sit amet leo ullamcorper ultricies. Fusce eget imperdiet ante. Pellentesque dapibus dignissim elit, id rutrum magna ullamcorper vitae.\n\nIn elementum dolor in mi placerat sollicitudin. Sed in quam quam, suscipit dignissim nisi. Suspendisse potenti. Sed nec iaculis justo. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nulla dignissim elit augue, eget hendrerit sem. Donec vel arcu est, vel pellentesque lectus. Nunc congue eleifend egestas. Donec sit amet cursus velit. Vestibulum eget nulla a enim egestas luctus et ac velit.\n\nLorem ipsum dolor sit amet, consectetur adipiscing elit. Aliquam erat volutpat. In nulla lorem, consectetur in pharetra sit amet, sagittis a neque. Sed lacinia, dolor sit amet scelerisque tincidunt, quam eros convallis ipsum, sit amet rutrum ligula purus quis nulla. Nunc quis nibh massa. Praesent vel augue risus. Cras commodo eleifend felis, interdum pharetra orci mattis sit amet. Cras lobortis, odio vel semper tempor, magna nunc aliquet dui, eu molestie tellus ligula et orci. Vestibulum pulvinar massa nec erat tempus a tincidunt arcu rhoncus. Morbi dictum sapien vel turpis consectetur ut mollis eros feugiat. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Etiam eget libero eget metus aliquam lobortis. Praesent feugiat tortor vel mi auctor a vehicula dolor pulvinar. Maecenas ac tortor velit, sit amet facilisis justo.");
    texite->SetWordwrap(true);
    texite->SetSize(16);
    texite->SetLayer(3);

    Frames::Frame *passover = Frames::Frame::CreateTagged(m_env->GetRoot());
    passover->SetPoint(Frames::X, 0.5, m_env->GetRoot(), 0.5, 0);
    passover->SetPoint(Frames::Y, 0.5, m_env->GetRoot(), 0.5, 0);
    passover->SetBackground(0, 1, 0, 0.1);
    passover->SetWidth(300);
    passover->SetHeight(300);
    passover->SetLayer(2);
  }
  void Core::l_shutdown() {
    lua_close(m_L);
    m_L = 0;
    
    delete m_env;
    m_env = 0;
  }

  int Core::l_register(lua_State *L) {
    lua_pushstring(L, "Glorp");
    lua_gettable(L, LUA_REGISTRYINDEX);

    lua_pushvalue(L, -2);
    int ref = luaL_ref(L, -2);
    lua_pop(L, 2);

    return ref;
  }
  void Core::l_retrieve(lua_State *L, int index) {
    assert(index != LUA_NOREF);

    lua_pushstring(L, "Glorp");
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
  void Core::l_callEvent(lua_State *L, int event) {
    l_retrieve(L, m_func_wrap);
    l_retrieve(L, event);
    if (lua_pcall(L, 1, 0, 0)) {
      dprintf("call event error");
      m_luaCrashed = true;
      CHECK(0);
      lua_pop(L, 1);
    }
  }
}
