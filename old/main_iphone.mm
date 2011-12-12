
#import <Foundation/NSBundle.h>
#import <Foundation/NSString.h>

#import <Foundation/Foundation.h>

#include <unistd.h>

#include <string>

#include "debug.h"

using namespace std;

void init_osx() {
  chdir([[[NSBundle mainBundle] bundlePath] UTF8String]);
}

#undef printf
void ods(const string &str) {
  if(str.size() && str[str.size() - 1] == '\n')
    NSLog(@"%s", str.c_str());
  else
    NSLog(@"%s\n", str.c_str());
}

void log_to_debugstring(const string &str) {
  if(str.size() && str[str.size() - 1] == '\n')
    NSLog(@"%s", str.c_str());
  else
    NSLog(@"%s\n", str.c_str());
  dbgrecord().push_back(str);
  if(dbgrecord().size() > 10000)
    dbgrecord().pop_front();
}
#define printf FAILURE
