
#include "os_ui.h"

#include "debug.h"

#import <AppKit/AppKit.h>
#import <Foundation/NSString.h>

// arrrrgh
NSAutoreleasePool *ffff;
class InitImportantStuff {
public:
  InitImportantStuff() {
    ffff = [[NSAutoreleasePool alloc] init];
    [NSApplication sharedApplication];
    
    ProcessSerialNumber psn = {0, kCurrentProcess};
    OSStatus returnCode = TransformProcessType(&psn, kProcessTransformToForegroundApplication);
    
    [NSApp activateIgnoringOtherApps:YES];
  }
  
  ~InitImportantStuff() {
    [ffff drain];
  }
} init_it;

int Message(const string &text, bool yesno) {
  dprintf("Message opening, %s\n", text.c_str());
  
  int rv;

  if(yesno) {
    rv = NSRunAlertPanel(@"Crash Report", [NSString stringWithCString:text.c_str() encoding:NSUTF8StringEncoding], @"OK", @"No", nil);
  } else {
    rv = NSRunAlertPanel(@"Crash Report", [NSString stringWithCString:text.c_str() encoding:NSUTF8StringEncoding], @"Done", nil, nil);
  }
  
  dprintf("Message closing\n");
  if(rv == NSAlertDefaultReturn)
    return true;
  return false;
}
