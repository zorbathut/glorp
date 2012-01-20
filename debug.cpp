
#include "debug.h"

#include "debug_911.h"
#include "os.h"
#include "args.h"

#include <stdarg.h>

using namespace std;

namespace Glorp {
  deque<string> &debugLog() {
    static deque<string> dbr;
    return dbr;
  }

  int rdprintf(const char *bort, ...) {
    // this is duplicated code with StringPrintf - I should figure out a way of combining these
    static vector< char > buf(1000);
    va_list args;
    buf[buf.size() - 1] = 1;

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
    
    Glorp::outputDebug(&(buf[0]));
    debugLog().push_back(&(buf[0]));
    
    if(debugLog().size() > 10000)
      debugLog().pop_front();
    
    return 0;
  };

  static bool first = true;
  void CheckHandler(const char *file, int line, const char *message) {
    if (first) {
      first = false;
      //stackOutput();
      Prepare911(file, line, message);
    }
  }
}
