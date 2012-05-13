#ifndef GLORP_PERFBAR
#define GLORP_PERFBAR

#include <boost/noncopyable.hpp>

using namespace std;

namespace Glorp {
  class PerfStack : boost::noncopyable {
  public:
    PerfStack(float r, float g, float b);
    ~PerfStack();
  };

  void perfbarReset();
  void perfbarDraw();
}

#endif
