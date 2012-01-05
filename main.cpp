
int debug_print(lua_State *L) {
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

void luainit(int argc, const char **argv) {
  L = lua_open();   /* opens Lua */
  luaL_openlibs(L);
  
  luaopen_lgl(L);
  luaopen_lal(L);
  
  lua_pushcfunction(L, debug_print);
  lua_setglobal(L, "print");
  
  lua_getglobal(L, "math");
  lua_pushcfunction(L, math_random);
  lua_setfield(L, "random");
  lua_pushcfunction(L, math_randomseed);
  lua_setfield(L, "randomseed");
  lua_pop(L, 1);

  // TODO: start killing all of this
  {
    using namespace luabind;
    
    luabind::open(L);
    
    module(L)
    [
      class_<WrappedTex, DontKillMeBro<WrappedTex> >("WrappedTex_Internal")
        .def("GetWidth", &WrappedTex::GetWidth)
        .def("GetHeight", &WrappedTex::GetHeight)
        .def("GetInternalWidth", &WrappedTex::GetInternalWidth)
        .def("GetInternalHeight", &WrappedTex::GetInternalHeight)
        .def("SetTexture", &WrappedTex::SetTexture)
        .def("GetPixel", &WrappedTex::GetPixel)
        .def(tostring(self))
        .def(const_self == other<const WrappedTex&>()),
      class_<PerfBarManager, DontKillMeBro<PerfBarManager> >("Perfbar_Init")
        .def(constructor<float, float, float>())
        .def("Destroy", &PerfBarManager::Destroy),
      def("Texture", &GetTex, adopt(result)),
      def("SetNoTexture", &SetNoTex),
      def("IsKeyDown", &IsKeyDownFrameAdapter),
      def("TriggerExit", &TriggerExit),
      def("GetMouseX", &gmx),
      def("GetMouseY", &gmy),
      def("ShowMouseCursor", &sms),
      def("LockMouseCursor", &lms),
      def("SetMousePosition", &setmousepos),
      #ifndef IPHONE
      def("ScreenshotTo", &screenshot_to),
      #endif
      def("TimeMicro", &time_micro),
      def("GetMidName", &get_mid_name),
      def("GetDesktopDirectory", &getDesktopDirectory),
      def("GetConfigDirectory", &getConfigDirectory),
      def("MakeConfigDirectory", &makeConfigDirectory),
      def("Perfbar_Set", &set_perfbar),
      def("get_stack_entry", &get_stack_entry),
      def("debugstack_annotated", &debugstack_annotated),
      
      #ifdef IPHONE
      def("touch_getCount", &os_touch_getCount),
      def("touch_getActive", &os_touch_getActive),
      def("touch_getX", &os_touch_getX),
      def("touch_getY", &os_touch_getY),
      #endif
      
      def("GetScreenX", &get_screenx),
      def("GetScreenY", &get_screeny),
      
      def("WindowInFocus", &window_in_focus),
      def("gluPerspective", &glewp)
      //def("CrashHorribly", &CrashHorribly)
    ];
    
    glorp_glutil_init(L);
    glorp_alutil_init(L);
    
    #ifdef GLORP_BOX2D
    glorp_box2d_init(L);
    #endif
    
    #ifdef GLORP_LFS
    luaopen_lfs(L);
    #endif
  }

  {
    int error = luaL_dofile(L, "data/glorp/wrap.lua");
    if(error) {
      error = luaL_dofile(L, "glorp/wrap.lua");
    }
    if(error) {
      CHECK(0, "%s", lua_tostring(L, -1));
    }
  }
  
  {
    lua_getglobal(L, "generic_wrap");
    lua_getglobal(L, "wrap_init");
    lua_pushstring(L, game_platform);
    lua_pushboolean(L, audio_enabled);
    for(int i = 0; i < argc; i++) {
      lua_pushstring(L, argv[i]);
    }
    int error = lua_pcall(L, 3 + argc, 0, 0);
    if(error) {
      CHECK(0, "%s", lua_tostring(L, -1));
    }
  }
  
  if(last_preserved_token.size()) {
    lua_getglobal(L, "generic_wrap");
    lua_getglobal(L, "de_fuckshit");
    lua_pushstring(L, last_preserved_token.c_str());
    int rv = lua_pcall(L, 2, 0, 0);
    if (rv) {
      CHECK(0, "%s", lua_tostring(L, -1));
    }
  }
}

void luashutdown() {
  dprintf("lua closing");
  lua_close(L);
  dprintf("lua closed");
  L = NULL;
}


void init()
{
  luainit(argc, argv);
}

void tick()
{
  {
    PerfStack pb(0.0, 0.0, 0.5);
    
    int thistick = system()->GetTime();
    lua_getglobal(L, "generic_wrap");
    lua_getglobal(L, "UI_Loop");
    lua_pushnumber(L, thistick - lasttick);
    lasttick = thistick;
    int rv = lua_pcall(L, 2, 0, 0);
    if (rv) {
      dprintf("Crash\n");
      dprintf("%s", lua_tostring(L, -1));
      meltdown();
      CHECK(0);
    }
  }
  
  // Do frame update here

  {
    PerfStack pb(0, 0.5, 0);
    lua_getglobal(L, "generic_wrap");
    lua_getglobal(L, "gcstep");
    int rv = lua_pcall(L, 1, 0, 0);
    if (rv) {
      dprintf("Crash\n");
      dprintf("%s", lua_tostring(L, -1));
      meltdown();
      CHECK(0);
    }
  }

  {
    vector<DontKillMeBro_KillerBase*> splatter;
    splatter.swap(to_be_killed);
    for(int i = 0; i < splatter.size(); i++)
      delete splatter[i];
  }
  
  glorp_glutil_tick();
  if (audio_enabled)
    glorp_alutil_tick();
  
  if(exiting) {
    OS::Shutdown();
  }
}

void key()
{
  // do key handling here
  
  
  // F12 keydown
  if(input()->IsKeyDownFrame(kKeyF12) && FLAGS_development) {
    if(!wasdown) {
      Teardown();
      init();
    }
    wasdown = true;
  } else {
    wasdown = false;
  }
  
}

void Teardown()
{
  meltdown();
  luashutdown();
  
  glorp_glutil_tick();
  if (audio_enabled)
    glorp_alutil_tick();
}







