
#include <sys/stat.h>

#include "debug.h"

namespace Glorp {
  #undef printf
  void outputDebug(const string &str) {
    printf("%s", str.c_str());
    if(!str.size() || str[str.size() - 1] != '\n')
      printf("\n");
  }
  #define printf FAILURE
  
  void crash() {
    raise(SIGKILL); // melt
    exit(-1);
  }
  
  string directoryDesktop() {
    // guess. seems to be generally accurate.
    string bf = getenv("HOME");
    return bf + "/Desktop/";
  }
  
  void spawn(const string &exec, const vector<string> &params) {
    vector<const char*> args;

    args.push_back(exec.c_str());
    for(int i = 0; i < params.size(); i++)
      args.push_back(params[i].c_str());
    args.push_back(NULL);
    

    // fork, then spawn.
    if(fork() == 0) {
      execv(exec.c_str(), const_cast<char * const *>(&args[0]));
      crash(); // should never get here
    }
  }
  
  // OSX, iPhone, make this work better? we do want a stack trace if at all possible
  typedef void (*sighandler_t)(int);

  class SignalHandler {
  public:
    static void signal_h(int signum) {
      disableStackTrace(); // we won't get anything anyway
      CHECK(0);
      crash();
    }
    
    SignalHandler() {
      signal(SIGSEGV, &signal_h);
    }
  } sighandler;

  int memory_usage() {
    return 0; // worry about this later
  }
  
  const unsigned int kStackTraceMax = 20;
  const unsigned int kMaxClassDepth = kStackTraceMax * 2 + 1;
  BOOST_STATIC_ASSERT(kMaxClassDepth % 2 == 1); // Must be odd.
   
  template <unsigned int S, unsigned int N = 1> struct StackTracer {
    static void printStack(vector<const void *> *vek) {
      if (!__builtin_frame_address(N))
        return;
   
      if (const void * const p = __builtin_return_address(N)) {
        vek->push_back(p);
        // Because this is recursive(ish), we may have to go down the stack by 2.
        StackTracer<S, N + S>::printStack(vek);
      }
    }
  };
   
  template <> struct StackTracer<1, kStackTraceMax> {
    static void printStack(vector<const void *> *vek) {}
  };

  template <> struct StackTracer<2, kMaxClassDepth> {
    static void printStack(vector<const void *> *vek) {}
  };

  vector<const void *> stackDump() {
    vector<const void *> stack;
    if(testInlined()) {
      dprintf("Stacktracing (inlined)\n");
      StackTracer<1>::printStack(&stack);
    } else {
      dprintf("Stacktracing\n");
      StackTracer<2>::printStack(&stack);
    }
    
    return stack;
  }
  
  static const string directory_delimiter = "/";

  string directoryConfig() {
    string bf = getenv("HOME");
    //dprintf("Home directory: %s\n", bf.c_str());
    return bf + "/." + game_fullname + "/";
  }

  void directoryMkdir(const string &str) {
    mkdir(str.c_str(), 0700);
  }

  string directoryTempfile() {
    char temparg[128] = "/tmp/";
    strcat(temparg, game_slug);
    strcat(temparg, "-XXXXXX");
    close(mkstemp(temparg));
    return temparg;
  }

  string directoryDelimiter() {
    return "/";
  }
}
