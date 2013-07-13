
#include "args.h"
#include "os.h"
#include "util.h"
#include "version.h"

#include <fstream>

#include <boost/assign.hpp>

using namespace std;

DEFINE_bool(report, true, "Attempt to report errors");

namespace Glorp {
  string writeDebug() {
    string writedest = directoryTempfile();
    
    ofstream ofs(writedest.c_str());
    if(!ofs)
      return "";
    
    deque<string> &debuglog = debugLog();
    for(int i = 0; i < debuglog.size(); i++) {
      while(debuglog[i].size() && debuglog[i][debuglog[i].size() - 1] == '\n')
        debuglog[i].erase(debuglog[i].begin() + debuglog[i].size() - 1);
      
      ofs << debugLog()[i] << '\n';
    }
    
    if(ofs)
      return writedest;
    else
      return "";
  };

  void Prepare911(const char *crashfname, int crashline, const char *message) {
    if(FLAGS_report) {
      string fname = writeDebug();
      if(!fname.size()) {
        dprintf("Failed to write file, aborting with a vengeance\n");
        return; // welp
      } else {
        dprintf("Wrote debug dump to %s\n", fname.c_str());
      }
      
      if(!message)
      {
        message = "";
      }
      
      vector<string> params;
      boost::assign::push_back(params)(Version::gameSlug)(fname)(string(Version::gameSlug) + "-" + Version::gameVersion + "-" + Version::gamePlatform)(crashfname)(Format("%d", crashline))(Format("%d", exeSize()))(message);
      //spawn("data/reporter", params);
      spawn("build/cygwin/reporter", params);
    }
  };
}
