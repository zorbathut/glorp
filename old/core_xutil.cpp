
#include "debug.h"

#include <Glop/Image.h>
#include <Glop/Os_Hacky.h>

#include <vector>

#include <climits>

using namespace std;

#ifdef LINUX  // only called in linux right now!
void glorp_set_icons() {
  /// ffffffffffffffffff
  Image *ico16 = Image::Load("data/mandible_icon-0.png");
  Image *ico32 = Image::Load("data/mandible_icon-1.png");
  Image *ico48 = Image::Load("data/mandible_icon-2.png");
  
  if(ico16 && ico32 && ico48) {
    Display *display = get_x_display();
    int screen = get_x_screen();
    Window window = get_x_window();
  
    Atom net_wm_icon = XInternAtom(display, "_NET_WM_ICON", False);
    Atom cardinal = XInternAtom(display, "CARDINAL", False);
      
    CHECK(ico16->GetWidth() == 16 && ico16->GetHeight() == 16);
    CHECK(ico32->GetWidth() == 32 && ico32->GetHeight() == 32);
    CHECK(ico48->GetWidth() == 48 && ico48->GetHeight() == 48);
    
    vector<unsigned long> dat;
    CHECK(sizeof(unsigned long) == 4);
    CHECK(CHAR_BIT == 8); // dear god I hope so
    
    for(int i = 0; i < 3; i++) {
      Image *cur = NULL;
      if(i == 0) cur = ico16;
      if(i == 1) cur = ico32;
      if(i == 2) cur = ico48;
      CHECK(cur);
      
      dat.push_back(cur->GetWidth());
      dat.push_back(cur->GetHeight());
      
      for(int y = 0; y < cur->GetHeight(); y++)
        for(int x = 0; x < cur->GetWidth(); x++)
          dat.push_back(*(unsigned long*)cur->Get(x, y));
    }
    
    XChangeProperty(display, window, net_wm_icon, cardinal, 32, PropModeReplace, (const unsigned char*)&dat[0], dat.size());
  }
  
  delete ico16;
  delete ico32;
  delete ico48;
}
#endif
