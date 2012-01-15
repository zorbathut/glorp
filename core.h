#ifndef GLORP_CORE
#define GLORP_CORE

#include "input.h"

#include <AL/al.h>
#include <AL/alc.h>

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
  };
}

#endif
