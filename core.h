#ifndef GLORP_CORE
#define GLORP_CORE

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

    lua_State *L;
    bool m_luaCrashed;

    void lua_init();
    void lua_shutdown();
  };
}

#endif
