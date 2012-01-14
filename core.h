#ifndef GLORP_CORE
#define GLORP_CORE

namespace Glorp {
  class Core {
  public:
    Core();
    ~Core();

    enum UpdateResult { UR_RENDER, UR_QUIT, UR_TICK };
    UpdateResult Update();
    
    void Render();
  };
}

#endif
