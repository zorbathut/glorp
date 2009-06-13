
#include "debug.h"

#include "debug_911.h"
#include "os.h"

#include <stdarg.h>

using namespace std;

int frameNumber = -1;
void *stackStart;

static vector<StackPrinter*> dbgstack;

StackPrinter::StackPrinter() {
  dbgstack.push_back(this);
}
StackPrinter::~StackPrinter() {
  CHECK(dbgstack.back() == this);
  dbgstack.pop_back();
}

void StackString::Print() const {
  dprintf("  %s", str_.c_str());
}
StackString::StackString(const string &str) : str_(str) {
  //dprintf("%s\n", str.c_str());
};

static bool do_stacktrace = true;
void disableStackTrace() {
  do_stacktrace = false;
}

void PrintDebugStack() {
  for(int i = (int)dbgstack.size() - 1; i >= 0; i--) {
    dprintf("Stack entry:\n");
    dbgstack[i]->Print();
  }
  dprintf("End of stack\n");
  if(do_stacktrace)
    dumpStackTrace();
}

vector<void (*)()> cfc;

void registerCrashFunction(void (*fct)()) {
  cfc.push_back(fct);
}
void unregisterCrashFunction(void (*fct)()) {
  CHECK(cfc.size());
  CHECK(count(cfc.begin(), cfc.end(), fct) >= 1);
  cfc.erase(find(cfc.begin(), cfc.end(), fct));
}

#ifdef VECTOR_PARANOIA
class VectorParanoiaChecker {
  static void throwshit() {
    throw int();
  }
public:
  VectorParanoiaChecker() {
    registerCrashFunction(&throwshit);
    
    {
      vector<int> test(100);
      try {
        dprintf("Testing vector paranoia, an error after this line is normal\n");
        test[100];
        unregisterCrashFunction(&throwshit);
        dprintf("VECTOR PARANOIA FAILED");
        CHECK(0);
      } catch (int x) {
        dprintf("Vector paranoia succeeded\n");
      }
    }
    
    {
      vector<bool> test(100);
      try {
        dprintf("Testing vector paranoia, an error after this line is normal\n");
        test[100];
        unregisterCrashFunction(&throwshit);
        dprintf("VECTOR PARANOIA FAILED");
        CHECK(0);
      } catch (int x) {
        dprintf("Vector paranoia succeeded\n");
        unregisterCrashFunction(&throwshit);
      }
    }
  }
} paranoia;
#endif

void CrashHandler(const char *fname, int line) {
  for(int i = cfc.size() - 1; i >= 0; i--)
    (*cfc[i])();
};

static bool inthread = false;

deque<string> &dbgrecord() {
  static deque<string> dbr;
  return dbr;
}

#ifdef SUPPRESS_GLOP

int dprintf(const char *bort, ...) {
  CHECK(!inthread);
  inthread = true;

  // this is duplicated code with StringPrintf - I should figure out a way of combining these
  static vector< char > buf(2);
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
      //assert(noresize == false);
      //assert(buf[buf.size() - 2] == 0);
      buf.resize(done + 2);
      done = -1;
      noresize = true;
    } else {
      //assert(done < (int)buf.size());
    }
    va_end(args);
  } while(done == buf.size() - 1 || done == -1);

  //assert(done < (int)buf.size());
  
  outputDebugString(&(buf[0]));
  dbgrecord().push_back(&(buf[0]));
  
  if(dbgrecord().size() > 10000)
    dbgrecord().pop_front();

  CHECK(inthread);
  inthread = false;
  
  return 0;
};

#endif

static bool reentering = false;
void HandleFailure(const char *file, int line) {
  if(reentering) {
    dprintf("Error in crash handling somewhere, terminating extremely abruptly");
  } else {
    reentering = true;
    dprintf("relp");
    PrintDebugStack();
    dprintf("pds");
    Prepare911(file, line);
    dprintf("911");
  }
  
  dprintf("seriouslycrash");
  seriouslyCrash();
}
