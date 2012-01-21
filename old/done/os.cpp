
#include "os.h"

#include "debug.h"
#include "util.h"
#include "parse.h"
#include "args.h"
#include "init.h"

#include <fstream>
#include <vector>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>

#include <boost/static_assert.hpp>

#define _WIN32_IE 0x0500 // ffff

using namespace std;

DEFINE_bool(addr2line, false, "Call addr2line for stack traces");

static string loc_exename;
void set_exename(int *argc, const char ***argv) {
  CHECK(*argc >= 1);
  loc_exename = "/proc/self/exe";

  struct stat lulz;
  if(stat(loc_exename.c_str(), &lulz)) {
    loc_exename = (*argv)[0];
  }
  
  if(stat(loc_exename.c_str(), &lulz)) {
    loc_exename += ".exe";  // lazy hack
  }
  
  exesize(); // check to make sure we can get it
}
  
ADD_INITTER(set_exename, -100);

int exesize() {
  CHECK(loc_exename.size());
  struct stat lulz;
  if(stat(loc_exename.c_str(), &lulz)) {
    dprintf("Couldn't stat file %s\n", loc_exename.c_str());
    CHECK(0);
  }
  return lulz.st_size;
}

#ifdef WIN32

#include <windows.h>
#include <shlobj.h>
#include <psapi.h>

void outputDebugString(const string &str) {
  OutputDebugString(str.c_str());
}

void seriouslyCrash() {
  //#ifdef NOEXIT
    TerminateProcess(GetCurrentProcess(), 1); // sigh
  //#endif
  exit(-1);
}

string getDesktopDirectory() {
  char meep[MAX_PATH + 1] = {0};
  SHGetFolderPath(NULL, CSIDL_DESKTOPDIRECTORY, NULL, SHGFP_TYPE_CURRENT, meep);
  return meep;
}

void SpawnProcess(const string &exec, const vector<string> &params) {
  dprintf("%s\n", exec.c_str());
  string texec = exec;

  for(int i = 0; i < params.size(); i++) {
    CHECK(count(params[i].begin(), params[i].end(), '"') == 0);
    texec += " \"" + params[i] + "\"";
  }

  vector<char> argh(texec.begin(), texec.end());
  argh.push_back('\0'); // why
  STARTUPINFO sinfo;
  memset(&sinfo, 0, sizeof(sinfo));
  sinfo.cb = sizeof(sinfo);
  PROCESS_INFORMATION pi;
  CHECK(CreateProcess(NULL, &argh[0], NULL, NULL, FALSE, 0, NULL, NULL, &sinfo, &pi));
  CloseHandle(&pi);
}

#else

#include <sys/stat.h>

#undef printf
void outputDebugString(const string &str) {
  printf("%s", str.c_str());
  if(!str.size() || str[str.size() - 1] != '\n')
    printf("\n");
}
#define printf FAILURE

void seriouslyCrash() {
  dprintf("crash1\n");
  raise(SIGKILL); // melt
  dprintf("crash2\n");
  exit(-1);
  dprintf("crash3\n");
}

string getDesktopDirectory() {
  string bf = getenv("HOME");
  return bf + "/Desktop/";
}

void SpawnProcess(const string &exec, const vector<string> &params) {
  vector<const char*> args;

  args.push_back(exec.c_str());
  for(int i = 0; i < params.size(); i++)
    args.push_back(params[i].c_str());
  args.push_back(NULL);
  
  dprintf("PARAMETERS\n");
  for(int i = 0; i < args.size() - 1; i++)
    dprintf("%s\n", args[i]);
  

  if(fork() == 0) {
    dprintf("trying to execv\n");
    execv(exec.c_str(), const_cast<char * const *>(&args[0]));  // stupid c
    dprintf("execv failed :(\n");
  }
}

#endif

string exename() {
  return loc_exename;
}

#ifdef WIN32
CONTEXT *crash_context = NULL;
class SignalHandler {
public:
  static WINAPI LONG signal_h(EXCEPTION_POINTERS *lol) {
    crash_context = lol->ContextRecord;
    CHECK(0);
  } // WE NEVER DIE
  
  SignalHandler() {
    SetUnhandledExceptionFilter(&signal_h);
  }
} sighandler;

int memory_usage() {
  PROCESS_MEMORY_COUNTERS pmc;
  GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
  return pmc.WorkingSetSize;
}

#else

// OSX, iPhone, make this work better? we do want a stack trace if at all possible
typedef void (*sighandler_t)(int);

class SignalHandler {
public:
  static void signal_h(int signum) {
    disableStackTrace(); // we won't get anything anyway
    CHECK(0);
  }
  
  SignalHandler() {
    signal(SIGSEGV, &signal_h);
  }
} sighandler;

int memory_usage() {
  return 0; // worry about this later
}

#endif

#ifdef __GNUG__
inline bool verifyInlined(const void *const p) {
  return __builtin_return_address(1) == p;
}

bool testInlined() {
  return verifyInlined(__builtin_return_address(1));
}

bool isUnoptimized() {
  return !testInlined();
}
#else
#error Not using GCC? What?
#endif

// if Windows
#ifdef WIN32
// magic!
#include <excpt.h>
#include <imagehlp.h>

vector<const void*> dumpy_stack() {
  vector<const void*> stack;
  
  CONTEXT *context = crash_context;
  
  CONTEXT ctx;
  if(!context) {
    // oh bloody hell
    HINSTANCE kernel32 = LoadLibrary("Kernel32.dll");
    typedef void ( * RtlCaptureContextFunc ) ( CONTEXT * ContextRecord );
    RtlCaptureContextFunc rtlCaptureContext = (RtlCaptureContextFunc) GetProcAddress( kernel32, "RtlCaptureContext" );
    
    rtlCaptureContext(&ctx);
    context = &ctx;
  }

  STACKFRAME frame;
  memset(&frame, 0, sizeof(frame));
  
  frame.AddrPC.Offset = context->Eip;
  frame.AddrPC.Mode = AddrModeFlat;
  frame.AddrStack.Offset = context->Esp;
  frame.AddrStack.Mode = AddrModeFlat;
  frame.AddrFrame.Offset = context->Ebp;
  frame.AddrFrame.Mode = AddrModeFlat;

  while(StackWalk(IMAGE_FILE_MACHINE_I386, GetCurrentProcess(), GetCurrentThread(), &frame, context, 0, SymFunctionTableAccess, SymGetModuleBase, 0)) {
    stack.push_back((const void*)frame.AddrPC.Offset);
  }

  return stack;
}
#else
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

vector<const void *> dumpy_stack() {
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

// this never happens anymore
/*
#else

void dumpStackTrace() {
  dprintf("No stack trace available\n");
}

bool isUnoptimized() {
  return false;
}*/

#endif

void dumpStackTrace() {
  vector<const void *> stack = dumpy_stack();
  
  vector<pair<string, string> > tokens;
  if(FLAGS_addr2line) {
    string line = "addr2line -f -e " + exename() + " ";
    for(int i = 0; i < stack.size(); i++)
      line += StringPrintf("%08x ", (unsigned int)stack[i]);
    line += "> addr2linetmp.txt";
    int rv = system(line.c_str());
    if(!rv) {
      {
        ifstream ifs("addr2linetmp.txt");
        string lin;
        while(getline(ifs, lin)) {
          string tlin;
          getline(ifs, tlin);
          tokens.push_back(make_pair(lin, tlin));
        }
      }
      unlink("addr2linetmp.txt");
    } else {
      dprintf("Couldn't call addr2line\n");
      return;
    }
  
    {
      string line = "c++filt -n -s gnu-v3 ";
      for(int i = 0; i < tokens.size(); i++)
        line += tokens[i].first + " ";
      line += "> cppfilttmp.txt";
      int rv = system(line.c_str());
      if(!rv) {
        {
          ifstream ifs("cppfilttmp.txt");
          string lin;
          int ct = 0;
          while(getline(ifs, lin)) {
            if(lin.size() && lin[0] == '_')
              lin.erase(lin.begin());
            dprintf("  %s - %s", tokens[ct].second.c_str(), lin.c_str());
            ct++;
          }
        }
        unlink("cppfilttmp.txt");
      } else {
        dprintf("Couldn't call c++filt\n");
        return;
      }
    }
  } else {
    for(int i = 0; i < stack.size(); i++)
      dprintf("  %08x", (unsigned int)stack[i]);
  }

  dprintf("\n");
}