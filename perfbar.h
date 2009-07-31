#ifndef DNET_PERFBAR
#define DNET_PERFBAR

#include <boost/noncopyable.hpp>

using namespace std;

class PerfStack : boost::noncopyable {
public:
  PerfStack(float r, float g, float b);
  ~PerfStack();
};

void startPerformanceBar();
void drawPerformanceBar();

#endif
