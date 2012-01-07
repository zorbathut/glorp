
#include "init.h"

#include <map>

using namespace std;

namespace Glorp {
  multimap<int, boost::function<void (int*, const char ***)> > &getFuncs() {
    static multimap<int, boost::function<void (int*, const char ***)> > it;
    return it;  // return it :D
  }

  void initProgram(int *argc, const char ***argv) {
    const multimap<int, boost::function<void (int*, const char ***)> > &mp = getFuncs();
    for(multimap<int, boost::function<void (int*, const char ***)> >::const_iterator itr = mp.begin(); itr != mp.end(); itr++) {
      itr->second(argc, argv);
    }
  }

  InitterRegister::InitterRegister(boost::function<void (int *, const char ***)> func, int priority) {
    getFuncs().insert(make_pair(priority, func));
  }
}
