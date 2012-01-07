
#include "util.h"

#include <cmath>
#include <sstream>
#include <algorithm>
#include <limits>

#include <stdarg.h>

using namespace std;

namespace Glorp {
  string Format(const char *bort, ...) {
    vector< char > buf(1000);
    va_list args;

    int done = 0;
    bool noresize = false;
    do {
      if(done && !noresize)
        buf.resize(buf.size() * 2);
      va_start(args, bort);
      done = vsnprintf(&(buf[0]), buf.size() - 1,  bort, args);
      if(done >= (int)buf.size()) {
        buf.resize(done + 2);
        done = -1;
        noresize = true;
      }
      va_end(args);
    } while(done == buf.size() - 1 || done == -1);

    CHECK(done < (int)buf.size());

    string rv = string(buf.begin(), buf.begin() + done);

    return rv;
  };
}
