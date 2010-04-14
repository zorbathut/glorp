
#include "args.h"
#include "os.h"
#include "util.h"
#include "version.h"

#include <fstream>

#include <boost/assign.hpp>

using namespace std;

DEFINE_bool(report, true, "Attempt to report errors");

string writeDebug() {
  string writedest = getTempFilename();
  
  ofstream ofs(writedest.c_str());
  if(!ofs)
    return "";
  
  for(int i = 0; i < dbgrecord().size(); i++) {
    while(dbgrecord()[i].size() && dbgrecord()[i][dbgrecord()[i].size() - 1] == '\n')
      dbgrecord()[i].erase(dbgrecord()[i].begin() + dbgrecord()[i].size() - 1);
    
    ofs << dbgrecord()[i] << '\n';
  }
  
  if(ofs)
    return writedest;
  else
    return "";
};

void Prepare911(const char *crashfname, int crashline) {
  if(FLAGS_report) {
    string fname = writeDebug();
    if(!fname.size()) {
      dprintf("Failed to write file, aborting with a vengeance\n");
      return; // welp
    } else {
      dprintf("Wrote debug dump to %s\n", fname.c_str());
    }
    
    vector<string> params;
    boost::assign::push_back(params)(game_slug)(fname)(string(game_slug) + "-" + game_version + "-" + game_platform)(crashfname)(StringPrintf("%d", crashline))(StringPrintf("%d", exesize()));
    SpawnProcess("reporter", params);
  }
};
