
#include "os_ui.h"

#include "debug.h"

#if defined(WIN32)

#include <windows.h> // :D

int Message(const string &text, bool yesno) {
  int rv = MessageBox(NULL, text.c_str(), "Crash Report", yesno ? MB_OKCANCEL : MB_OK);
  if(rv == IDOK)
    return true;
  return false;
}

#elif defined(MACOSX)

// implemented in os_ui_osx.mm

#else

#include <FL/fl_ask.H>

int Message(const string &text, bool yesno) {
  if(!yesno) {
    fl_message("%s", text.c_str());
    return false;
  } else {
    return fl_ask("%s", text.c_str());
  }
}

#endif

