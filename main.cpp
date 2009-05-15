
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

class Text : public Destroyable<FancyTextFrame> {
public:
  Text(const std::string &x) : Destroyable<FancyTextFrame>(x) { };
};

void Init(const string &title) {
  // Initialize
  LogToFunction(&log_to_debugstring);
  System::Init();

  window()->SetTitle(title);
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
    CHECK((font = GradientFont::Load("Sansation_Regular.ttf", 1.0f, 0.5f, -0.3f, 1.0f)) != 0);
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
