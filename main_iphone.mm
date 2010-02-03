
#import <Foundation/NSBundle.h>
#import <Foundation/NSString.h>

#include <unistd.h>

void init_osx() {
  chdir([[[NSBundle mainBundle] bundlePath] UTF8String]);
}
