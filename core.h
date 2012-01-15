#ifndef GLORP_CORE
#define GLORP_CORE

#include "input.h"

namespace Glorp {
  class Core {
  public:
    Core();
    ~Core();

    void Event(const KeyEvent &event);

    enum UpdateResult { UR_RENDER, UR_QUIT, UR_TICK };
    UpdateResult Update();
    
    void Render();
  };
}

#endif
