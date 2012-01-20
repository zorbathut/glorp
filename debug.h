#ifndef GLORP_DEBUG
#define GLORP_DEBUG

#include <stdio.h>  // we include this because it makes its own dprintf function that we cheerfully nuke

#include <deque>
#include <string>

using namespace std;

namespace Glorp
{
  deque<string> &debugLog();
  void CheckHandler(const char *file, int line, const char *message);
  int rdprintf(const char *format, ...) __attribute__((format(printf,1,2)));
  int rdprintf();
}

#define dprintf(format, args...) Glorp::rdprintf("%16.16s:%4d: " format, __FILE__, __LINE__, ## args)
#define CHECK(x, args...) (__builtin_expect(!!(x), 1) ? (void)(1) : (dprintf("Error at %s:%d - %s\n", __FILE__, __LINE__, #x), dprintf("" args), Glorp::CheckHandler(__FILE__, __LINE__, 0)))
#define printf FAILURE

#endif
