
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

#include <iostream>

#include <lua.hpp>

#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#include <luabind/discard_result_policy.hpp>

#include "debug.h"
#include "util.h"

using namespace std;

void log_to_debugstring(const string &str) {
  OutputDebugString(str.c_str());
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
  acul += "\n";
  log_to_debugstring(acul);
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
  int error = luaL_dofile(L, file);
  if (error) {
    dprintf("%s", lua_tostring(L, -1));
    lua_pop(L, 1);  /* pop error message from the stack */
    CHECK(0);
  }
}

void printit(int q) {
  dprintf("teh number is %d", q);
}


TableauFrame *world;

template<typename Sub> class Destroyable : public Sub {
  private:
    ListId id;
  
  public:
    Destroyable<Sub>() {
      id = world->AddChild(this);
    }
    Destroyable<Sub>(const std::string &x) : Sub(x) {
      id = world->AddChild(this);
    }
    ~Destroyable<Sub>() {
      world->RemoveChildNoDelete(id);
    }
      
    void Move(float x, float y) {
      world->MoveChild(id, x, y);
    }
};

class Sprite : public Destroyable<GlopFrame> {
private:
  friend std::ostream& operator<<(std::ostream &, const Sprite &);
  string icon;

  float x;
  float y;
  float dx;
  float dy;
  float ts;
  float size;
  Texture *tex;

public:
  
  void Render() const {
    float lx = x + dx * (system()->GetTime() - ts) / 1000;
    float ly = y + dy * (system()->GetTime() - ts) / 1000;
    
    float ts = size * 1024 / 2;
    int sx = (int)(lx * 1024 - ts);
    int sy = (int)(ly * 768 - ts);
    int ex = (int)(lx * 1024 + ts);
    int ey = (int)(ly * 768 + ts);
        
    GlUtils2d::RenderTexture(sx, sy, ex, ey, tex);
  }
  
  Sprite(const string &image) {
    tex = Texture::Load("art/" + image);
    CHECK(tex);
  }
  
  void Move(float in_x, float in_y, float in_dx, float in_dy, float in_size) {
    x = in_x;
    y = in_y;
    dx = in_dx;
    dy = in_dy;
    ts = system()->GetTime();
    size = in_size;
  }
  
  ~Sprite() {
    delete tex;
  }
};

class Ability : public Destroyable<GlopFrame> {
private:
  friend std::ostream& operator<<(std::ostream &, const Ability &);
  string icon;

  float x;
  float y;
  float size;
  Texture *tex;

  float gcd;

public:
  
  void Render() const {
    float ts = size * 1024 / 2;
    int sx = (int)(x * 1024 - ts);
    int sy = (int)(y * 768 - ts);
    int ex = (int)(x * 1024 + ts);
    int ey = (int)(y * 768 + ts);
    
    /*
    int old_clipping_enabled = glIsEnabled(GL_SCISSOR_TEST), old_scissor_test[4];
    if (old_clipping_enabled)
      glGetIntegerv(GL_SCISSOR_BOX, old_scissor_test);
    else
      glEnable(GL_SCISSOR_TEST);
    glScissor(sx, GetWindow()->GetHeight() - sy, ex - sx, ey - sy);*/
    
    GlUtils2d::RenderTexture(sx, sy, ex, ey, tex);
    
    if(gcd > 0) {
      float gc = gcd * 8;
      
      glCullFace(GL_FRONT);
      glBegin(GL_TRIANGLE_FAN);
      glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
      glVertex2f((sx + ex) / 2, (sy + ey) / 2);
      glVertex2f((sx + ex) / 2, sy);
      
      float xs[] = {0.5, 0, 0, 0, 0.5, 1, 1, 1, 0.5};
      float ys[] = {0, 0, 0.5, 1, 1, 1, 0.5, 0, 0};
      
      for(int i = 1; i <= 8; i++) {
        float tcl = min(gc, 1.f);
        float x = sx + (ex - sx) * (xs[i - 1] * (1 - tcl) + xs[i] * tcl);
        float y = sy + (ey - sy) * (ys[i - 1] * (1 - tcl) + ys[i] * tcl);
        glVertex2f(x, y);
        gc = gc - 1;
        if(gc < 0) break;
      }
      
      glEnd();
      glCullFace(GL_BACK);
      
      glBegin(GL_TRIANGLE_FAN);
      glColor4f(0.0f, 0.0f, 0.0f, 0.2f);
      glVertex2f(ex, ey);
      glVertex2f(sx, ey);
      glVertex2f(sx, sy);
      glVertex2f(ex, sy);
      glEnd();
    }
    
    /*
    // Disable clipping
    if (!old_clipping_enabled)
      glDisable(GL_SCISSOR_TEST);
    else
      glScissor(old_scissor_test[0], old_scissor_test[1], old_scissor_test[2], old_scissor_test[3]);*/
  }
  
  void SetGCD(float in_gcd) {
    gcd = in_gcd;
  }
  
  Ability(const string &image) {
    tex = Texture::Load("art/" + image);
    CHECK(tex);
  }
  
  void Move(float in_x, float in_y, float in_size) {
    x = in_x;
    y = in_y;
    size = in_size;
  }
  
  ~Ability() {
    delete tex;
  }
};
std::ostream& operator<<(std::ostream &ostr, const Ability &gidg) {
  ostr << "(Ability " << gidg.icon << ")";
  return ostr;
}

class Text : public Destroyable<FancyTextFrame> {
public:
  Text(const std::string &x) : Destroyable<FancyTextFrame>(x) { };
};

class Gidget : public Destroyable<SingleParentFrame> {
private:
  friend std::ostream& operator<<(std::ostream &, const Gidget &);

  TableauFrame *tf;
  
  string name;

  FancyTextFrame *status;

  boost::optional<luabind::object> callback;

  void RecomputeSize(int rec_width, int rec_height) {
    SingleParentFrame::RecomputeSize(window()->GetWidth() / 10, window()->GetHeight() / 6);
  }
  
  void Initit(const string &in_name, boost::optional<luabind::object> func) {
    name = in_name;
    callback = func;
    
    tf = new TableauFrame();
    SetChild(tf);
    
    tf->AddChild(new HollowBoxFrame(kWhite));
    {
      ListId tfidx = tf->AddChild(new TextFrame(name, kWhite));
      tf->MoveChild(tfidx, 0.5, 0.1);
    }
      
    {
      status = new FancyTextFrame("", GuiTextStyle(kWhite, 0.02));
      ListId tfidx = tf->AddChild(status);
      tf->MoveChild(tfidx, 0.5, 0.6);
    }
  }
  
  bool OnKeyEvent(const KeyEvent &event, bool gained_focus) {
    if(!callback)
      return false;
    if(!event.IsNonRepeatPress())
      return false;
    if(!IsPointVisible(input()->GetMouseX(), input()->GetMouseY()))
      return false;
    
    const GlopKey &ki = event.GetMainKey();
    
    if(ki.IsMouseMotion())
      return false;
    
    string kiout;
    
    if(ki == kMouseLButton) kiout = "lbutton";
    else if(ki.IsKeyboardKey()) kiout += (char)ki.index;
    
    if(kiout.size()) {
      try {
        bool grrr = luabind::call_function<bool>(*callback, kiout);
      } catch(luabind::error &e) {
        lua_State* L = e.state();
        dprintf("%s", lua_tostring(L, -1));
        lua_pop(L, 1);  /* pop error message from the stack */
        CHECK(0);
      }
    }
    
    return false;
  }
public:
  Gidget(const string &in_name, const luabind::object &event_call) {
    Initit(in_name, event_call);
  };
  Gidget(const string &in_name) {
    Initit(in_name, boost::optional<luabind::object>());
  };
  
  void ChangeStatus(const string &name) {
    status->SetText(name);
  }
      
};
std::ostream& operator<<(std::ostream &ostr, const Gidget &gidg) {
  ostr << "(Gidget " << gidg.name << ")";
  return ostr;
}

int main() {
  // Initialize
  LogToFunction(&log_to_debugstring);
  System::Init();

  window()->SetTitle("lol");
  window()->SetVSync(true);
  ASSERT(window()->Create(1024, 768, false));
  
  lua_State *L = lua_open();   /* opens Lua */
  luaL_openlibs(L);
  dprintf("rega");
  lua_register(L, "print", debug_print);
  dprintf("regb");
  
  {
    using namespace luabind;
    
    luabind::open(L);
    
    module(L)
    [
      def("printit", &printit),
      class_<Gidget>("Gidget_Make")
        .def(constructor<const string &, const luabind::object &>())
        .def(constructor<const string &>())
        .def("Move", &Gidget::Move)
        .def("ChangeStatus", &Gidget::ChangeStatus)
        .def("Hide", &Gidget::Hide)
        .def(tostring(self)),
      class_<Ability>("Ability_Make")
        .def(constructor<const string &>())
        .def("Move", &Ability::Move)
        .def("SetGCD", &Ability::SetGCD)
        .def(tostring(self)),
      class_<Sprite>("Sprite_Make")
        .def(constructor<const string &>())
        .def("Move", &Sprite::Move)
        .def("Hide", &Sprite::Hide),
      class_<Text>("Text_Make")
        .def(constructor<const std::string &>())
        .def("Move", &Text::Move)
        .def("Hide", &Text::Hide)
        .def("SetText", &Text::SetText)
    ];
  }
  
  {
    Font *font;
    CHECK((font = GradientFont::Load("Calibri.ttf", 1.0f, 0.5f, -0.3f, 1.0f)) != 0);
    InitDefaultFrameStyle(font);
  }
  
  world = new TableauFrame();
  FocusFrame *everything = new FocusFrame(world);
  window()->AddFrame(everything);
  
  loadfile(L, "main.lua");
  
  int lasttick = system()->GetTime();
  while(window()->IsCreated()) {
    system()->Think();
    
    int thistick = system()->GetTime();
    lua_getglobal(L, "loop");
    lua_pushnumber(L, thistick - lasttick);
    lasttick = thistick;
    int rv = lua_pcall(L, 1, 0, 0);
    if (rv) {
      dprintf("%s", lua_tostring(L, -1));
      CHECK(0, "%s", lua_tostring(L, -1));
      lua_pop(L, 1);  /* pop error message from the stack */
    }
  }
  
  dprintf("exiting");
}
