
#include "perfbar.h"

#include "args.h"
#include "debug.h"
#include "os.h"
#include "util.h"

#include "GL/glew.h"

#include <stack>

using namespace std;

DEFINE_bool(perfbar, false, "Enable performance bar");

namespace Glorp {
  class PerfChunk {
  public:
    double start;
    double end;
    float r, g, b;
    int indent;
  };

  double start;
  static vector<PerfChunk> pchunks;

  static stack<pair<const PerfStack *, PerfChunk> > pstack;

  PerfStack::PerfStack(float r, float g, float b) {
    PerfChunk pch;
    pch.start = timeMicro();
    pch.r = r;
    pch.g = g;
    pch.b = b;
    pch.indent = pstack.size();
    pstack.push(make_pair(this, pch));
  }
  PerfStack::~PerfStack() {
    if(pstack.top().first != this) {
      dprintf("Horribly broken perfstack! Something is very wrong. Oh dearie, dearie me.\n");
      return;
    }
    PerfChunk pch = pstack.top().second;
    pstack.pop();
    pch.end = timeMicro();
    pchunks.push_back(pch);
  }

  void perfbarReset() {
    if(!pstack.empty()) {
      dprintf("Horribly broken perfstack! Something is very wrong. Oh dearie, dearie me.\n");
      pstack = stack<pair<const PerfStack *, PerfChunk> >();  // bizarre syntax needed because stack doesn't have .clear()
    }
    pchunks.clear();
    start = timeMicro();
  }

  void perfbarDraw() {
    #ifndef IPHONE
      CHECK(pstack.empty());
      
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glOrtho(0, 1, 1, 0, -1, 1);
      
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
      
      glBindTexture(GL_TEXTURE_2D, 0);
      
      glBegin(GL_QUADS);
      
      const float xmarg = 0.01;
      const float xwid = xmarg * 0.9;
      const float xstart = 1 - xmarg;
      
      int megindent = 0;
      
      const float scale = 60.f * (60.f / 70) / 1000000;
      
      for(int i = 0; i < pchunks.size(); i++) {
        double xps = xstart - xmarg * pchunks[i].indent;
        double ys = (pchunks[i].start - start) * scale;
        double ye = (pchunks[i].end - start) * scale;
        
        glColor3f(pchunks[i].r, pchunks[i].g, pchunks[i].b);
        glVertex2f(xps, ys);
        glVertex2f(xps - xwid, ys);
        glVertex2f(xps - xwid, ye);
        glVertex2f(xps, ye);
        
        megindent = max(megindent, pchunks[i].indent + 1);
      }
      
      glColor3f(1.0, 1.0, 1.0);
      
      float xpind = xstart - xmarg * megindent;
      glVertex2f(1, (1000000.f / 60) * scale);
      glVertex2f(1, (1000000.f / 60) * scale + xwid);
      glVertex2f(xpind, (1000000.f / 60) * scale + xwid);
      glVertex2f(xpind, (1000000.f / 60) * scale);
      
      
      // now let's print the mem bar as well
      float memheight = memoryUsage() / 200000000.;
      float mems = 1 - xwid / 4;
      float meme = 1 - xwid * 3 / 4;
      glColor3f(1.0, 1.0, 1.0);
      glVertex2f(mems, 0);
      glVertex2f(meme, 0);
      glVertex2f(meme, memheight);
      glVertex2f(mems, memheight);
      
      glEnd();
    #endif
  }
}

