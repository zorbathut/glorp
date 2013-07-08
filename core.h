#ifndef GLORP_CORE
#define GLORP_CORE

#include "frame/environment.h"
#include "frame/input.h"

#include <AL/al.h>
#include <AL/alc.h>

#include <lua.hpp>

namespace Glorp {
  class Core {
  public:
    Core();
    ~Core();

    static bool Prestartup();

    void Input(const Frame::InputEvent &event);

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

    // functions
    void l_registerInspect(lua_State *L, const char *name, int (*func)(lua_State *));

    static int l_system_time_real(lua_State *L);

    // Event tables here
    int l_registerEvent(lua_State *L, const char *event);
    void l_callEvent(lua_State *L, int event, int params);

    int m_func_wrap;

    int m_event_system_update_begin;
    int m_event_system_update_end;

    int m_event_system_mouse;
    int m_event_system_key_down;
    int m_event_system_key_up;
    int m_event_system_key_type;
    int m_event_system_key_repeat;
    
    Frame::Environment *m_env;
  };
}

#endif
