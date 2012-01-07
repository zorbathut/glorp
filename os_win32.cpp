
#include "os.h"
#include "debug.h"
#include "version.h"

#define _WIN32_IE 0x0500 // ffff

#include <windows.h>
#include <shlobj.h>
#include <psapi.h>
#include <excpt.h>
#include <imagehlp.h>
#include <direct.h>

#include <algorithm>

using namespace std;

namespace Glorp {
  void outputDebug(const string &str) {
    OutputDebugString(str.c_str());
  }
  
  void crash() {
    TerminateProcess(GetCurrentProcess(), 1); // sigh. this should not be necessary. on cygwin, sometimes it is. fuck cygwin.
    exit(-1);
  }
  
  string directoryDesktop() {
    char meep[MAX_PATH + 1] = {0};
    SHGetFolderPath(NULL, CSIDL_DESKTOPDIRECTORY, NULL, SHGFP_TYPE_CURRENT, meep);
    return meep;
  }
  
  void spawn(const string &exec, const vector<string> &params) {
    string texec = exec;

    for(int i = 0; i < params.size(); i++) {
      CHECK(count(params[i].begin(), params[i].end(), '"') == 0);
      texec += " \"" + params[i] + "\"";
    }

    STARTUPINFO sinfo;
    memset(&sinfo, 0, sizeof(sinfo));
    sinfo.cb = sizeof(sinfo);
    
    PROCESS_INFORMATION pi;
    CHECK(CreateProcess(NULL, (char*)texec.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &sinfo, &pi));
    CloseHandle(&pi);
  }
  
  CONTEXT *crash_context = NULL;
  class SignalHandler {
  public:
    static WINAPI LONG signal_h(EXCEPTION_POINTERS *lol) {
      crash_context = lol->ContextRecord;
      CHECK(0);
      crash();
    } // WE NEVER DIE
    
    SignalHandler() {
      SetUnhandledExceptionFilter(&signal_h);
    }
  } sighandler;

  int memoryUsage() {
    PROCESS_MEMORY_COUNTERS pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
    return pmc.WorkingSetSize;
  }
  
  vector<const void*> stackDump() {
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
  
  static const string directory_delimiter = "\\";

  string directoryConfig() {
    char buff[MAX_PATH + 1];
    SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, buff);
    //dprintf("Home directory: %s\n", buff);
    return string(buff) + "\\" + Version::gameFullname + "\\";
  }

  void directoryMkdir(const string &str) {
    _mkdir(str.c_str());
  }

  string directoryTempfile() {
    char buff[MAX_PATH + 1];
    GetTempPath(sizeof(buff), buff);
    char fname[MAX_PATH + 1];
    GetTempFileName(buff, Version::gameSlug, 0, fname);
    return fname;
  }
  
  string directoryDelimiter() {
    return "\\";
  }
}
