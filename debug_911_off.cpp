
#include "os.h"

namespace Glorp {
  void Prepare911(const char *fname, int line, const char *message) {
    crash(); // DENY
  };
}
