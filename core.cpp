
#include <Glop/Base.h>
#include <Glop/Font.h>
#include <Glop/GlopFrame.h>
#include <Glop/GlopWindow.h>
#include <Glop/Image.h>
#include <Glop/Input.h>
#include <Glop/OpenGl.h>
#include <Glop/System.h>
#include <Glop/Thread.h>
#include <Glop/glop3d/Camera.h>
#include <Glop/glop3d/Mesh.h>
#include <Glop/Sound.h>

#include <iostream>

#include <lua.hpp>

#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#include <luabind/discard_result_policy.hpp>

#include "debug.h"
#include "util.h"
#include "core.h"
#include "args.h"
#include "init.h"
#include "perfbar.h"

#include "LuaGL.h"

using namespace std;

lua_State *L;

const int virt_width = 640;
const int virt_height = 480;

void ods(const string &str) {
  OutputDebugString(str.c_str());
}

void log_to_debugstring(const string &str) {
  OutputDebugString(str.c_str());
  dbgrecord().push_back(str);
  if(dbgrecord().size() > 10000)
    dbgrecord().pop_front();
}

extern "C" {
static int debug_print(lua_State *L) {
  string acul = "";
  int n = lua_gettop(L);  /* number of arguments */
  int i;
  lua_getglobal(L, "tostring");
  for (i=1; i<=n; i++) {
    const char *s;
    lua_pushvalue(L, -1);  /* function to be called */
    lua_pushvalue(L, i);   /* value to print */
    lua_call(L, 1, 1);
    s = lua_tostring(L, -1);  /* get result */
    if (s == NULL)
      return luaL_error(L, LUA_QL("tostring") " must return a string to "
                           LUA_QL("print"));
    if (i>1) acul += "\t";
    acul += s;
    lua_pop(L, 1);  /* pop result */
  }
  dprintf("%s", acul.c_str());
  return 0;
}
}

luaL_Reg regs[] = {
  {"printy", debug_print},
  {NULL, NULL},
};

void stackDump (lua_State *L) {
  dprintf("stackdump");
  int i;
  int top = lua_gettop(L);
  for (i = 1; i <= top; i++) {  /* repeat for each level */
    int t = lua_type(L, i);
    switch (t) {

      case LUA_TSTRING:  /* strings */
        dprintf("`%s'", lua_tostring(L, i));
        break;

      case LUA_TBOOLEAN:  /* booleans */
        dprintf("%s", lua_toboolean(L, i) ? "true" : "false");
        break;

      case LUA_TNUMBER:  /* numbers */
        dprintf("%g", lua_tonumber(L, i));
        break;

      default:  /* other values */
        dprintf("%s", lua_typename(L, t));
        break;

    }
    dprintf("  ");  /* put a separator */
  }
  dprintf("\n");  /* end the listing */
}

void loadfile(lua_State *L, const char *file) {
  lua_getglobal(L, "core__loadfile");
  lua_pushstring(L, file);
  int rv = lua_pcall(L, 1, 0, 0);
  if (rv) {
    CHECK(0, "%s", lua_tostring(L, -1));
  }
}


TableauFrame *world;

#if 0
class Receiver : public TableauFrame {
  bool OnKeyEvent(const KeyEvent &event, bool gained_focus) {
    if(!event.IsPress() && !event.IsRelease())
      return false;
    
    const GlopKey &ki = event.GetMainKey();
    
    if(ki.IsMouseMotion())
      return false;
    
    string kiout;
    
    if(ki == kMouseLButton) kiout = "lbutton";
    else if(ki == kKeyLeft) kiout = "arrow_left";
    else if(ki == kKeyRight) kiout = "arrow_right";
    else if(ki == kKeyUp) kiout = "arrow_up";
    else if(ki == kKeyDown) kiout = "arrow_down";
    else if(ki.IsKeyboardKey()) kiout += (char)ki.index;
    
    if(kiout.size()) {
      lua_getglobal(L, "input");
      lua_pushlstring(L, kiout.c_str(), kiout.size());
      if(event.IsPress())
        lua_pushstring(L, "press");
      else
        lua_pushstring(L, "release");
      int rv = lua_pcall(L, 2, 0, 0);
      if (rv) {
        dprintf("%s", lua_tostring(L, -1));
        CHECK(0, "%s", lua_tostring(L, -1));
      }
    }
    
    return false;
  }
};
#endif

template<typename Sub> class Destroyable : public Sub {
  private:
    ListId id;
  
  public:
    Destroyable<Sub>() {
      id = world->AddChild(this);
    }
    template <typename T> Destroyable<Sub>(const T &x) : Sub(x) {
      id = world->AddChild(this);
    }
    ~Destroyable<Sub>() {
      world->RemoveChildNoDelete(id);
    }
      
    void Move(float x, float y) {
      world->MoveChild(id, x, y);
    }
    
    void SetLayer(int layer) {
      world->MoveChild(id, layer);
    }
};

class Layer : public Destroyable<GlopFrame> {
  int layer;
public:
  
  void Render() const {
    PerfStack pb(0.5, 0.0, 0.0);
    
    lua_getglobal(L, "generic_wrap");
    lua_getglobal(L, "render");
    lua_pushnumber(L, layer);
    int rv = lua_pcall(L, 2, 0, 0);
    if (rv) {
      dprintf("%s", lua_tostring(L, -1));
      CHECK(0);
    }
  }
  
  Layer(int lay) : Destroyable<GlopFrame>() {
    SetLayer(lay);
    layer = lay;
  }
};

map<int, Layer *> layers;


class Perfbar : public GlopFrame {
public:
  
  void Render() const {
    drawPerformanceBar();
    startPerformanceBar();
  }
};


map<string, Texture *> images;
Texture *getTex(const string &image) {
  Texture *tex;
  if(images.count(image)) {
    tex = images[image];
  } else {
    Image *img = Image::Load("data/" + image + ".png");
    CHECK(img, image.c_str());
    
    for(int y = 0; y < img->GetHeight(); y++)
      for(int x = 0; x < img->GetWidth(); x++)
        if(*(unsigned long*)img->Get(x, y) == 0xffffffff)
          *(unsigned long*)img->Get(x, y) = 0;
    
    tex = new Texture(img);   // we leak some stuff here
    images[image] = tex;
  }
  
  return tex;
}

class WrappedTex {
private:
  Texture *tex;

public:
  WrappedTex(const string &t) {
    tex = getTex(t);
    CHECK(tex, "%s", t.c_str());
  }
    
  int GetWidth() const {
    return tex->GetWidth();
  }
  int GetHeight() const {
    return tex->GetHeight();
  }
  int GetInternalWidth() const {
    return tex->GetInternalWidth();
  }
  int GetInternalHeight() const {
    return tex->GetInternalHeight();
  }
  
  void SetTexture() {
    GlUtils::SetTexture(tex);
    
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    //glTranslatef(-1 / (float)tex->GetInternalWidth() / 2, -1 / (float)tex->GetInternalHeight() / 2, 0);
    glMatrixMode(GL_MODELVIEW);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  }
};

void SetNoTex() {
  GlUtils::SetNoTexture();
  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
}

void RegisterRenderLayer(int lay) {
  if(!layers[lay]) {
    layers[lay] = new Layer(lay);
  }
}

float cvx(float x) {
  return x / virt_width * window()->GetWidth();
}
float cvy(float y) {
  return y / virt_height * window()->GetHeight();
}

class Sprite : public Destroyable<GlopFrame> {
private:
  friend std::ostream& operator<<(std::ostream &, const Sprite &);
  string icon;

  float sx;
  float sy;
  float ex;
  float ey;
  Texture *tex;

public:
  
  void Render() const {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    if(sx <= ex) {
      GlUtils2d::RenderTexture((int)cvx(sx), (int)cvy(sy), (int)cvx(ex), (int)cvy(ey), tex);
    } else {
      GlUtils2d::RenderTexture((int)cvx(sx), (int)cvy(sy), (int)cvx(ex), (int)cvy(ey), float(tex->GetWidth()) / tex->GetInternalWidth(), 0, 0, float(tex->GetHeight()) / tex->GetInternalHeight(), true, tex);
    }
  }
  
  Sprite(const string &image) {
    tex = getTex(image);

    CHECK(tex, "%s\n", image.c_str());
  }
  
  void Move(float in_sx, float in_sy, float in_ex, float in_ey) {
    sx = in_sx;
    ex = in_ex;
    sy = in_sy;
    ey = in_ey;
  }
};

class Text : public Destroyable<FancyTextFrame> {
public:
  void ScaledMove(float x, float y) {
    //dprintf("%f/%f convert to %f/%f", x, y, x / virt_width, y / virt_height);
    //dprintf("old was %f/%f", world->GetChildRelX(id), world->GetChildRelY(id));
    Move(x / virt_width, y / virt_height);
  }
  void WrappedText(const string &txt) {
    SetText("\1Cffffffff\1" + txt);
  }
  
  void Render() const {
    Destroyable<FancyTextFrame>::Render();
  }
  
  Text(const string &txt) : Destroyable<FancyTextFrame>("hi hi hi hi hi") {
    WrappedText(txt);
  };
  
  ~Text() { 
    dprintf("Obliterating text");
  }
};

class Fader : public Destroyable<SolidBoxFrame> {
public:
  Fader() : Destroyable<SolidBoxFrame>(Color(0, 0, 0, 0)) { };
  
  void SetOpacity(float opacity) {
    SetColor(Color(0, 0, 0, opacity));
  }
};

GlopKey adapt(const string &id) {
  CHECK(id.size() > 0);
  
  GlopKey ki;
  
  if(id == "arrow_left") ki = kKeyLeft; else
  if(id == "arrow_right") ki = kKeyRight; else
  if(id == "arrow_up") ki = kKeyUp; else
  if(id == "arrow_down") ki = kKeyDown; else
  if(id == "enter") ki = kKeyEnter; else
    ki = GlopKey(id[0]);
  
  return ki;
}

bool IsKeyDownFrameAdapter(const string &id) {
  return input()->IsKeyDownFrame(adapt(id));
};

bool WasKeyPressedAdapter(const string &id) {
  return input()->WasKeyPressed(adapt(id), false);
};

map<string, SoundSample *> sounds;

vector<SoundSource> ss;
void DoASound(const string &sname) {
  for(int i = 0; i < ss.size(); i++) {
    if(ss[i].IsStopped()) {
      ss[i].Stop();
      ss.erase(ss.begin() + i);
    }
  }
  if(!sounds[sname]) {
    sounds[sname] = SSLoad(sname);
  }
  CHECK(sounds[sname]);
  SoundSource nss = sounds[sname]->Play();
  //dprintf("%d, %d\n", nss.IsPaused(), nss.IsStopped());
  //CHECK(!nss.IsPaused() && !nss.IsStopped(), "%d, %d\n", nss.IsPaused(), nss.IsStopped());
  ss.push_back(nss);
};

class PerfBarManager {
  PerfStack *ps;
  
public:
  PerfBarManager(float r, float g, float b) {
    ps = new PerfStack(r, g, b);
  }
  
  void Destroy() {
    delete ps;
    ps = NULL;
  }
};

void TriggerExit() {
  window()->Destroy();
};

void luainit() {
  L = lua_open();   /* opens Lua */
  luaL_openlibs(L);
  luaopen_opengl(L);
  lua_register(L, "print", debug_print);
  
  {
    using namespace luabind;
    
    luabind::open(L);
    
    module(L)
    [
      class_<Sprite>("Sprite_Make")
        .def(constructor<const string &>())
        .def("Move", &Sprite::Move)
        .def("SetLayer", &Sprite::SetLayer)
        .def("Hide", &Sprite::Hide)
        .def("Show", &Sprite::Show),
      class_<Text>("Text_Make")
        .def(constructor<const std::string &>())
        .def("Move", &Text::ScaledMove)
        .def("Hide", &Text::Hide)
        .def("Show", &Text::Show)
        .def("SetLayer", &Text::SetLayer)
        .def("SetText", &Text::WrappedText),
      class_<Fader>("Fader_Make")
        .def(constructor<>())
        .def("SetOpacity", &Fader::SetOpacity)
        .def("Hide", &Fader::Hide)
        .def("Show", &Fader::Show)
        .def("SetLayer", &Fader::SetLayer),
      class_<WrappedTex>("Texture")
        .def(constructor<const std::string &>())
        .def("GetWidth", &WrappedTex::GetWidth)
        .def("GetHeight", &WrappedTex::GetHeight)
        .def("GetInternalWidth", &WrappedTex::GetInternalWidth)
        .def("GetInternalHeight", &WrappedTex::GetInternalHeight)
        .def("SetTexture", &WrappedTex::SetTexture),
      class_<PerfBarManager>("Perfbar_Init")
        .def(constructor<float, float, float>())
        .def("Destroy", &PerfBarManager::Destroy),
      def("SetNoTexture", &SetNoTex),
      def("IsKeyDownFrame", &IsKeyDownFrameAdapter),
      def("WasKeyPressed_Frame", &WasKeyPressedAdapter),
      def("PlaySound", &DoASound),
      def("TriggerExit", &TriggerExit),
      def("RegisterRenderLayer", &RegisterRenderLayer)
    ];
  }
    
  int error = luaL_dofile(L, "data/wrap.lua");
  if(error) {
    error = luaL_dofile(L, "glorp/wrap.lua");
  }
  if(error) {
    CHECK(0, "%s", lua_tostring(L, -1));
  }
    
  loadfile(L, "main.lua");
}
void luashutdown() {
  dprintf("lua closing");
  lua_close(L);
  dprintf("lua closed");
  L = NULL;
  
  for(map<int, Layer*>::iterator itr = layers.begin(); itr != layers.end(); itr++)
    delete itr->second;
  layers.clear();
}

void glorp_init(const string &name, const string &fontname, int width, int height, int argc, const char **argv) {
  
  //dprintf("inity");
  // Initialize
  LogToFunction(&log_to_debugstring);
  System::Init();

  setInitFlagFile("glorp/settings");
  initProgram(&argc, const_cast<const char ***>(&argv));
  
  window()->SetTitle(name);
  window()->SetVSync(true);
  
  {
    GlopWindowSettings gws;
    gws.min_aspect_ratio = (float)width / height;
    gws.min_inverse_aspect_ratio = (float)height / width;
    ASSERT(window()->Create(width, height, false, gws));
    window()->SetVSync(false);
  }
  
  {
    Font *font = ShadowFont::Load(fontname.c_str(), 0.05, 0.05);
    CHECK(font);
    InitDefaultFrameStyle(font);
  }
  
  world = new TableauFrame();
  {
    ListId fid = world->AddChild(new Perfbar);
    world->MoveChild(fid, 100000);
  }
  FocusFrame *everything = new FocusFrame(world);
  window()->AddFrame(everything);
  
  luainit();
  
  int lasttick = system()->GetTime();
  while(window()->IsCreated()) {
    system()->Think();
    
    {
      PerfStack pb(0.0, 0.0, 0.5);
      
      int thistick = system()->GetTime();
      lua_getglobal(L, "generic_wrap");
      lua_getglobal(L, "loop");
      lua_pushnumber(L, thistick - lasttick);
      lasttick = thistick;
      int rv = lua_pcall(L, 2, 0, 0);
      if (rv) {
        CHECK(0, "%s", lua_tostring(L, -1));
      }
    }
    
    if(input()->IsKeyDownFrame(kKeyF12)) {
      luashutdown();
      luainit();
    }
  }
  
  luashutdown();
  
  for(map<string, Texture *>::const_iterator itr = images.begin(); itr != images.end(); itr++)
    delete itr->second;
  
  dprintf("exiting");
}

SoundSample *SSLoad(const string &fname_base) {
  SoundSample *rv;
  rv = SoundSample::Load("data/" + fname_base + ".ogg");
  if(rv) return rv;
  rv = SoundSample::Load("data/" + fname_base + ".wav");
  if(rv) return rv;
  CHECK(0, fname_base.c_str());
}
