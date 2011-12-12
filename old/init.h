#ifndef DNET_INIT
#define DNET_INIT






#include <boost/function.hpp>

using namespace std;

void initProgram(int *argc, const char ***argv);

class InitterRegister {
public:
  InitterRegister(boost::function<void (int *, const char ***)> func, int priority);
};

#define ADD_INITTER(func, priority)   namespace { InitterRegister inr##__LINE__(func, priority); }

#endif
