
#include "perfbar.h"
#include "debug.h"

#include <Glop/OpenGl.h>

#include <Glop/System.h>

#include <stack>

using namespace std;

//DEFINE_bool(perfbar, false, "Enable performance bar");

class PerfChunk {
public:
  float start;
  float end;
  float r, g, b;
  int indent;
};

float start;
static vector<PerfChunk> pchunks;

static stack<pair<const PerfStack *, PerfChunk> > pstack;

PerfStack::PerfStack(float r, float g, float b) {
  PerfChunk pch;
  pch.start = system()->GetTimeMicro();
  pch.r = r;
  pch.g = g;
  pch.b = b;
  pch.indent = pstack.size();
  pstack.push(make_pair(this, pch));
}
PerfStack::~PerfStack() {
  CHECK(pstack.top().first == this);
  PerfChunk pch = pstack.top().second;
  pstack.pop();
  pch.end = system()->GetTimeMicro();
  pchunks.push_back(pch);
}

void startPerformanceBar() {
  CHECK(pstack.empty());
  pchunks.clear();
  start = system()->GetTimeMicro();
}

void drawPerformanceBar() {
  CHECK(pstack.empty());
  
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, 1, 1, 0, -1, 1);
  
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  GlUtils::SetNoTexture();
  
  glBegin(GL_QUADS);
  
  const float xmarg = 0.01;
  const float xwid = xmarg * 0.9;
  const float xstart = 1 - xmarg;
  
  int megindent = 0;
  
  const float scale = 60.f * (60.f / 70) / 1000000;
  
  for(int i = 0; i < pchunks.size(); i++) {
    float xps = xstart - xmarg * pchunks[i].indent;
    float ys = (pchunks[i].start - start) * scale;
    float ye = (pchunks[i].end - start) * scale;
    
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
  
  glEnd();
}
