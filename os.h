
#include <string>
#include <vector>

using namespace std;

namespace Glorp {
  // debug-type stuff
  void outputDebug(const string &str);
  void crash() __attribute__ ((noreturn));
  void spawn(const string &exec, const vector<string> &params);
  vector<const void*> stackDump();
  void stackOutput();
  int exeSize();
  
  // directories/fs
  string directoryDesktop();
  string directoryConfig();
  void directoryConfigMake();
  string directoryTempfile();
  void directoryMkdir(const string &str);
  string directoryDelimiter();
  
  // performance
  int memoryUsage();
  bool isUnoptimized();

  // OS commands
  void beginShutdown();
}
