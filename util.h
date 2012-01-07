#ifndef DNET_UTIL
#define DNET_UTIL

#include "debug.h"

#include <vector>

using namespace std;

namespace Glorp {
  /*************
   * Text processing
   */

  #ifdef printf
  #define PFDEFINED
  #undef printf
  #endif

  string Format(const char *bort, ...) __attribute__((format(printf,1,2)));

  #ifdef PFDEFINED
  #define printf FAILURE
  #undef PFDEFINED
  #endif
}

#endif
