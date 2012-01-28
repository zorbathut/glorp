#ifndef GLORP_CORE
#define GLORP_CORE

// so it doesn't catch the printf define
#include <frames/environment.h>

#include "input.h"

#include <AL/al.h>
#include <AL/alc.h>

#include <lua.hpp>

namespace Glorp {
  class Core {
  public:
    Core();
    ~Core();

    static bool Prestartup();

    void Event(const KeyEvent &event);

    enum UpdateResult { UR_RENDER, UR_QUIT, UR_TICK };
    UpdateResult Update();
    
    void Render();

  private:
    ALCdevice* m_alcDevice;
    ALCcontext* m_alcContext;

    bool m_audioEnabled;

    lua_State *m_L;
    bool m_luaCrashed;

    void l_init();
    void l_shutdown();

    int l_register(lua_State *L);
    void l_retrieve(lua_State *L, int id);

    // Event tables here
    int l_registerEvent(lua_State *L, const char *event);
    void l_callEvent(lua_State *L, int event);

    int m_func_wrap;

    int m_event_system_update_begin;
    int m_event_system_update_end;

    int m_event_system_mouse;
    int m_event_system_key;
    
    Frames::Environment *m_env;
  };
}

#endif
