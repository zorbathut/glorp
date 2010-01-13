
#include "LuaGL.h"
#include "LuaGL_ext.h"

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

#include <boost/random.hpp>

#include "debug.h"
#include "util.h"
#include "core.h"
#include "args.h"
#include "init.h"
#include "perfbar.h"
#include "version.h"
#include "os.h"

#include <png.h>

using namespace std;

lua_State *L;

const int virt_width = 640;
const int virt_height = 480;

#ifdef WIN32
  void ods(const string &str) {
    OutputDebugString(str.c_str());
  }

  void log_to_debugstring(const string &str) {
    OutputDebugString(str.c_str());
    dbgrecord().push_back(str);
    if(dbgrecord().size() > 10000)
      dbgrecord().pop_front();
  }
#endif

#ifdef MACOSX
  #undef printf
  void ods(const string &str) {
    if(str.size() && str[str.size() - 1] == '\n')
      printf("%s", str.c_str());
    else
      printf("%s\n", str.c_str());
  }

  void log_to_debugstring(const string &str) {
    if(str.size() && str[str.size() - 1] == '\n')
      printf("%s", str.c_str());
    else
      printf("%s\n", str.c_str());
    dbgrecord().push_back(str);
    if(dbgrecord().size() > 10000)
      dbgrecord().pop_front();
  }
  #define printf FAILURE
#endif

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


string last_preserved_token;

void meltdown() {
  lua_getglobal(L, "generic_wrap");
  lua_getglobal(L, "fuckshit");
  int rv = lua_pcall(L, 1, 1, 0);
  if (rv) {
    CHECK(0, "%s", lua_tostring(L, -1));
  }
  
  if(lua_isstring(L, -1)) {
    last_preserved_token = lua_tostring(L, -1);
  }
  lua_pop(L, 1);
}

TableauFrame *world;

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
public:
  
  void Render() const {
    PerfStack pb(0.5, 0.0, 0.0);
    
    lua_getglobal(L, "generic_wrap");
    lua_getglobal(L, "render");
    int rv = lua_pcall(L, 1, 0, 0);
    if (rv) {
      dprintf("%s", lua_tostring(L, -1));
      meltdown();
      CHECK(0);
    }
  }
};

map<int, Layer *> layers;

bool perfbar_enabled = false;
void set_perfbar(bool x) {perfbar_enabled = x;}
class Perfbar : public GlopFrame {
public:
  
  void Render() const {
    if(perfbar_enabled)
      drawPerformanceBar();
    startPerformanceBar();
  }
};
Layer *uilayer = NULL;


class WrappedTex {
private:
  Texture *tex;
  Image *img;
  string fname;

public:
  WrappedTex(Image *intex, const string &inf) {
    img = intex;
    tex = new Texture(img);
    fname = inf;
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
    glTranslatef(-1 / (float)tex->GetInternalWidth() / 2, -1 / (float)tex->GetInternalHeight() / 2, 0);
    glMatrixMode(GL_MODELVIEW);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  }
  
  const string &getfname() const {
    return fname;
  }
  
  ~WrappedTex() {
    delete tex;
    delete img;
  };
};
std::ostream& operator<<(std::ostream&ostr, WrappedTex const&ite) {
  ostr << "<texture \"" << ite.getfname() << ">";
  return ostr;
}


WrappedTex *GetTex(const string &image) {
  Image *tex = Image::Load("data/" + image + ".png");
  if(!tex) return NULL;
  
  return new WrappedTex(tex, image);
}

void SetNoTex() {
  GlUtils::SetNoTexture();
  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
}

float cvx(float x) {
  return x / virt_width * window()->GetWidth();
}
float cvy(float y) {
  return y / virt_height * window()->GetHeight();
}

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
  if(id == "mouse_left") ki = kMouseLButton; else
  if(id == "mouse_right") ki = kMouseRButton; else
  if(id == "mouse_middle") ki = kMouseMButton; else
  if(id == "mouse_wheel_up") ki = kMouseWheelUp; else
  if(id == "mouse_wheel_down") ki = kMouseWheelUp; else
  if(id == "shift") ki = kKeyLeftShift; else
  if(id == "shift_left") ki = kKeyLeftShift; else
  if(id == "shift_right") ki = kKeyRightShift; else
  if(id == "ctrl") ki = kKeyLeftControl; else
  if(id == "printscreen") ki = kKeyPrintScreen; else
  if(id == "escape") ki = kKeyEscape; else
    ki = GlopKey(id[0]);
  
  return ki;
}

bool IsKeyDownFrameAdapter(const string &id) {
  return input()->IsKeyDownFrame(adapt(id));
};

bool WasKeyPressedAdapter(const string &id) {
  return input()->WasKeyPressed(adapt(id), false);
};

class KeyList : public KeyListener {
  void OnKeyEvent(const KeyEvent &event) {
    string keyvent;
    if(event.GetMainKey() == kMouseLButton) { keyvent = "mouse_left"; }
    if(event.GetMainKey() == kMouseRButton) { keyvent = "mouse_right"; }
    if(event.GetMainKey() == kMouseMButton) { keyvent = "mouse_middle"; }
    if(event.GetMainKey() == kMouseWheelUp) { keyvent = "mouse_wheel_up"; }
    if(event.GetMainKey() == kMouseWheelDown) { keyvent = "mouse_wheel_down"; }
    
    if(event.GetMainKey() == kKeyLeft) { keyvent = "arrow_left"; }
    if(event.GetMainKey() == kKeyRight) { keyvent = "arrow_right"; }
    if(event.GetMainKey() == kKeyUp) { keyvent = "arrow_up"; }
    if(event.GetMainKey() == kKeyDown) { keyvent = "arrow_down"; }
    
    if(event.GetMainKey().IsKeyboardKey() && event.GetMainKey().index >= 'a' && event.GetMainKey().index <= 'z') { keyvent = string(1, event.GetMainKey().index); }
    if(event.GetMainKey().IsKeyboardKey() && event.GetMainKey().index >= '0' && event.GetMainKey().index <= '9') { keyvent = string(1, event.GetMainKey().index); }
    if(event.GetMainKey().IsKeyboardKey() && event.GetMainKey().index == '`') { keyvent = string(1, event.GetMainKey().index); }
    
    if(event.GetMainKey() == kKeyF2) { keyvent = "f2"; }
    if(event.GetMainKey() == kKeyDelete) { keyvent = "delete"; }
    if(event.GetMainKey() == kKeyPrintScreen) { keyvent = "printscreen"; }
    
    if(event.GetMainKey() == kKeyEnter) { keyvent = "enter"; }
    if(event.GetMainKey() == kKeyEscape) { keyvent = "escape"; }
    
    unsigned char aski = input()->GetAsciiValue(event.GetMainKey());
    string bleep;
    if(aski) {
      bleep = string(1, aski);
    }
    
    if(keyvent.size() || aski) {
      string typ;
      if(event.IsDoublePress()) {
        typ = "press_double";
      } else if(event.IsRepeatPress()) {
        typ = "press_repeat";
      } else if(event.IsNonRepeatPress()) {
        typ = "press";
      } else if(event.IsRelease()) {
        typ = "release";
      } else if(event.IsNothing()) {
        typ = "nothing";
      }
      
      lua_getglobal(L, "generic_wrap");
      lua_getglobal(L, "UI_Key");
      if(keyvent.size()) {
        lua_pushstring(L, keyvent.c_str());
      } else {
        lua_pushnil(L);
      }
      if(bleep.size()) {
        lua_pushstring(L, bleep.c_str());
      } else {
        lua_pushnil(L);
      }
      lua_pushstring(L, typ.c_str());
      int rv = lua_pcall(L, 4, 0, 0);
      if (rv) {
        dprintf("%s", lua_tostring(L, -1));
        meltdown();
        CHECK(0);
      }
    }
  }
};

map<pair<string, float>, SoundSample *> sounds;

vector<SoundSource> ss;
SoundSource SoundCore(const string &sname, float vol, bool loop) {
  vol = ceil(vol * 16) / 16;
  for(int i = 0; i < ss.size(); i++) {
    if(ss[i].IsStopped()) {
      ss[i].Stop();
      ss.erase(ss.begin() + i);
    }
  }
  if(!sounds[make_pair(sname, vol)]) {
    sounds[make_pair(sname, vol)] = SSLoad(sname, vol);
  }
  
  if(!sounds[make_pair(sname, vol)]) {
    meltdown();
    CHECK(sounds[make_pair(sname, vol)]);
  }
  
  //dprintf("Looping: %d\n", loop);
  return sounds[make_pair(sname, vol)]->Play(loop);
  //dprintf("%d, %d\n", nss.IsPaused(), nss.IsStopped());
  //CHECK(!nss.IsPaused() && !nss.IsStopped(), "%d, %d\n", nss.IsPaused(), nss.IsStopped());
};

void DoASound(const string &sname, float vol) {
  ss.push_back(SoundCore(sname, vol, false));
}

SoundSource ControllableSound(const string &sname, float vol, bool loop) {
  return SoundCore(sname, vol, loop);
}

void clean_up_sounds() {
  for(int i = 0; i < ss.size(); i++)
    ss[i].Stop();
  ss.clear();
  
  for(map<pair<string, float>, SoundSample *>::iterator itr = sounds.begin(); itr != sounds.end(); itr++)
    delete itr->second;
  sounds.clear();
}

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

bool exiting = false;
void TriggerExit() {
  exiting = true;
};

vector<int> to_delete_lists;
vector<int> to_delete_shaders;
vector<int> to_delete_programs;

class GlListID {
  int id;
  
public:
  GlListID() {
    id = glGenLists(1);
  }
  ~GlListID() {
    to_delete_lists.push_back(id);
  }
  
  int get() const {
    return id;
  }
};

class GlShader {
  int id;
  
public:
  GlShader(string typ) {
    int typi;
    if(typ == "VERTEX_SHADER") typi = GL_VERTEX_SHADER; else
    if(typ == "FRAGMENT_SHADER") typi = GL_FRAGMENT_SHADER; else
      assert(0);
    id = glCreateShader(typi);
  }
  ~GlShader() {
    to_delete_shaders.push_back(id);
  }
  
  int get() const {
    return id;
  }
};

class GlProgram {
  int id;
  
public:
  GlProgram() {
    id = glCreateProgram();
  }
  ~GlProgram() {
    to_delete_programs.push_back(id);
  }
  
  int get() const {
    return id;
  }
};

void adaptaload(const string &fname) {
  int error = luaL_dofile(L, ("data/" + fname).c_str());
  if(error) {
    error = luaL_dofile(L, ("glorp/" + fname).c_str());
  }
  if(error) {
    CHECK(0, "%s", lua_tostring(L, -1));
  }
}

void adaptaload_wrapped(const string &fname, int argc, const char **argv) {
  int error = luaL_dostring(L, ("file_temp, file_err = loadfile(\"data/" + fname + "\"); assert(file_temp, file_err)").c_str());
  if(error) {
    error = luaL_dostring(L, ("file_temp, file_err = loadfile(\"glorp/" + fname + "\"); assert(file_temp, file_err)").c_str());
  }
  if(error) {
    error = luaL_dostring(L, ("file_temp, file_err = loadfile(\"" + fname + "\"); assert(file_temp, file_err)").c_str());
  }
  
  if(!error) {
    lua_getglobal(L, "generic_wrap");
    lua_getglobal(L, "file_temp");
    for(int i = 0; i < argc; i++) {
      lua_pushstring(L, argv[i]);
    }
    error = lua_pcall(L, 1 + argc, 0, 0);
    if(!error) {
      error = luaL_dostring(L, "file_temp, file_err = nil, nil");
    }
  }
  
  if(error) {
    CHECK(0, "%s", lua_tostring(L, -1));
  }
}

int gmx() {return input()->GetMouseX();};
int gmy() {return input()->GetMouseY();};

void tsc(TextFrame *tf, float r, float g, float b, float a) {
  tf->SetColor(Color(r, g, b, a));
}

// arrrgh
boost::lagged_fibonacci9689 rngstate(time(NULL));
static int math_random (lua_State *L) {
  /* the `%' avoids the (rare) case of r==1, and is needed also because on
     some systems (SunOS!) `rand()' may return a value larger than RAND_MAX */
  lua_Number r = rngstate();
  switch (lua_gettop(L)) {  /* check number of arguments */
    case 0: {  /* no arguments */
      lua_pushnumber(L, r);  /* Number between 0 and 1 */
      break;
    }
    case 1: {  /* only upper limit */
      int u = luaL_checkint(L, 1);
      luaL_argcheck(L, 1<=u, 1, "interval is empty");
      lua_pushnumber(L, floor(r*u)+1);  /* int between 1 and `u' */
      break;
    }
    case 2: {  /* lower and upper limits */
      int l = luaL_checkint(L, 1);
      int u = luaL_checkint(L, 2);
      luaL_argcheck(L, l<=u, 2, "interval is empty");
      lua_pushnumber(L, floor(r*(u-l+1))+l);  /* int between `l' and `u' */
      break;
    }
    default: return luaL_error(L, "wrong number of arguments");
  }
  return 1;
}
static int math_randomseed (lua_State *L) {
  rngstate = boost::lagged_fibonacci9689(luaL_checkint(L, 1));
  return 0;
}

void sms(bool bol) {
  input()->ShowMouseCursor(bol);
}

void get_stack_entry(lua_State *L, int level) {
  lua_pushliteral(L, "");
  
  lua_Debug ar;
  CHECK(lua_getstack(L, level, &ar));
  
  lua_getinfo(L, "Snl", &ar);
  
  lua_pushfstring(L, "%s:", ar.short_src);
  if (ar.currentline > 0)
    lua_pushfstring(L, "%d:", ar.currentline);

  if (*ar.namewhat != '\0')  /* is there a name? */
      lua_pushfstring(L, " in function " LUA_QS, ar.name);
  else {
    if (*ar.what == 'm')  /* main? */
      lua_pushfstring(L, " in main chunk");
    else if (*ar.what == 'C' || *ar.what == 't')
      lua_pushliteral(L, " ?");  /* C function or tail call */
    else
      lua_pushfstring(L, " in function <%s:%d>",
                         ar.short_src, ar.linedefined);
  }
  lua_concat(L, lua_gettop(L) - 1);
}

bool screenshot_to(const string &fname) {
  FILE *fil = fopen(fname.c_str(), "wb");
  if(!fil) return false;
  
  // alright let's get this shit done
  glReadBuffer(GL_FRONT);
  vector<char> dat;
  int x = window()->GetWidth();
  int y = window()->GetHeight();
  dprintf("screeny %dx%d\n", x, y);
  dat.resize(x * y * 4);
  vector<char*> ptrs;
  for(int i = 0; i < y; i++)
    ptrs.push_back(&dat[(y - i - 1) * x * 4]);
  glReadPixels(0, 0, x, y, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, &dat[0]);
  
  png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  png_infop info_ptr = png_create_info_struct(png_ptr);
  assert(!setjmp(png_jmpbuf(png_ptr)));
  png_init_io(png_ptr, fil);
  
  png_set_IHDR(png_ptr, info_ptr, x, y, 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
  png_write_info(png_ptr, info_ptr);
  png_write_image(png_ptr, (png_byte**)&ptrs[0]); // yaaaaaay
  png_write_end(png_ptr, NULL);
  
  png_destroy_write_struct(&png_ptr, &info_ptr);
  
  fclose(fil);
  return true;
}

string get_mid_name() {
  return game_midname;
}

#define LEVELS1	12	/* size of the first part of the stack */
#define LEVELS2	10	/* size of the second part of the stack */

void debugstack_annotated(lua_State *L) {
  int level = 1;
  
  lua_pushliteral(L, "stack traceback:");
  
  lua_Debug ar;
  int firstpart = 1;  /* still before eventual `...' */
  while (lua_getstack(L, level++, &ar)) {
    if (level > LEVELS1 && firstpart) {
      /* no more than `LEVELS2' more levels? */
      if (!lua_getstack(L, level+LEVELS2, &ar))
        level--;  /* keep going */
      else {
        lua_pushliteral(L, "\n\t...");  /* too many levels */
        while (lua_getstack(L, level+LEVELS2, &ar))  /* find last levels */
          level++;
      }
      firstpart = 0;
      continue;
    }
    lua_pushliteral(L, "\n\t");
    lua_getinfo(L, "Snl", &ar);
    lua_pushfstring(L, "%s:", ar.short_src);
    if (ar.currentline > 0)
      lua_pushfstring(L, "%d:", ar.currentline);
    if (*ar.namewhat != '\0')  /* is there a name? */
        lua_pushfstring(L, " in function " LUA_QS, ar.name);
    else {
      if (*ar.what == 'm')  /* main? */
        lua_pushfstring(L, " in main chunk");
      else if (*ar.what == 'C' || *ar.what == 't')
        lua_pushliteral(L, " ?");  /* C function or tail call */
      else
        lua_pushfstring(L, " in function <%s:%d>",
                           ar.short_src, ar.linedefined);
    }
    
    lua_getlocal(L, &ar, 1);
    if(lua_istable(L, -1)) {
      lua_pushliteral(L, "__name");
      lua_gettable(L, -2);
      if(lua_isstring(L, -1)) {
        lua_pushliteral(L, " in frame (");
        lua_insert(L, -2);
        lua_pushliteral(L, ")");
        lua_concat(L, 3);
        
        lua_insert(L, -2);
      } else {
        lua_pop(L, 1);
      }
    }
    lua_pop(L, 1);
    
    lua_concat(L, lua_gettop(L));
  }
  lua_concat(L, lua_gettop(L));
}


#define ll_subregister(L, cn, sn, f) (lua_getglobal(L, cn), lua_pushstring(L, sn), lua_pushcfunction(L, f), lua_settable(L, -3))

void luainit(int argc, const char **argv) {
  L = lua_open();   /* opens Lua */
  luaL_openlibs(L);
  luaopen_opengl(L);
  luaopen_opengl_ext(L);
  lua_register(L, "print", debug_print);
  
  ll_subregister(L, "math", "random", math_random);
  ll_subregister(L, "math", "randomseed", math_randomseed);
  
  {
    using namespace luabind;
    
    luabind::open(L);
    
    module(L)
    [
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
      class_<WrappedTex>("WrappedTex_Internal")
        //.def(constructor<const std::string &>())
        .def("GetWidth", &WrappedTex::GetWidth)
        .def("GetHeight", &WrappedTex::GetHeight)
        .def("GetInternalWidth", &WrappedTex::GetInternalWidth)
        .def("GetInternalHeight", &WrappedTex::GetInternalHeight)
        .def("SetTexture", &WrappedTex::SetTexture)
        .def(tostring(self)),
      class_<PerfBarManager>("Perfbar_Init")
        .def(constructor<float, float, float>())
        .def("Destroy", &PerfBarManager::Destroy),
      class_<TextFrame>("TextFrame_Make")
        .def(constructor<const std::string &>())
        .def("GetX", &TextFrame::GetX)
        .def("GetY", &TextFrame::GetY)
        .def("GetClipX1", &TextFrame::GetClipX1)
        .def("GetClipY1", &TextFrame::GetClipY1)
        .def("GetClipX2", &TextFrame::GetClipX2)
        .def("GetClipY2", &TextFrame::GetClipY2)
        .def("GetWidth", &TextFrame::GetWidth)
        .def("GetHeight", &TextFrame::GetHeight)
        .def("SetPosition", &TextFrame::SetPosition)
        .def("UpdateSize", &TextFrame::UpdateSize)
        .def("Render", &TextFrame::Render)
        .def("SetTextSize", &TextFrame::SetTextSize)
        .def("SetColor", &TextFrame::SetColor)
        .def("SetText", &TextFrame::SetText)
        .def("GetText", &TextFrame::GetText),
      class_<FancyTextFrame>("FancyTextFrame_Make")
        .def(constructor<const std::string &>())
        .def("GetX", &TextFrame::GetX)
        .def("GetY", &TextFrame::GetY)
        .def("GetClipX1", &TextFrame::GetClipX1)
        .def("GetClipY1", &TextFrame::GetClipY1)
        .def("GetClipX2", &TextFrame::GetClipX2)
        .def("GetClipY2", &TextFrame::GetClipY2)
        .def("SetPosition", &FancyTextFrame::SetPosition)
        .def("UpdateSize", &FancyTextFrame::UpdateSize)
        .def("Render", &FancyTextFrame::Render)
        .def("SetText", &FancyTextFrame::SetText)
        .def("GetText", &FancyTextFrame::GetText),
      class_<SoundSource>("SourceSource_Make")
        .def("Stop", &SoundSource::Stop),
      class_<GlListID>("GlListID")
        .def(constructor<>())
        .def("get", &GlListID::get),
      class_<GlShader>("GlShader")
        .def(constructor<const std::string &>())
        .def("get", &GlShader::get),
      class_<GlProgram>("GlProgram")
        .def(constructor<>())
        .def("get", &GlProgram::get),
      def("Texture", &GetTex),
      def("Text_SetColor", &tsc),
      def("SetNoTexture", &SetNoTex),
      def("IsKeyDownFrame", &IsKeyDownFrameAdapter),
      def("WasKeyPressed_Frame", &WasKeyPressedAdapter),
      def("PlaySound_Core", &DoASound),
      def("ControlSound_Core", &ControllableSound),
      def("TriggerExit", &TriggerExit),
      def("GetMouseX", &gmx),
      def("GetMouseY", &gmy),
      def("ShowMouseCursor", &sms),
      def("ScreenshotTo", &screenshot_to),
      def("GetMidName", &get_mid_name),
      def("GetDesktopDirectory", &getDesktopDirectory),
      def("GetConfigDirectory", &getConfigDirectory),
      def("MakeConfigDirectory", &makeConfigDirectory),
      def("Perfbar_Set", &set_perfbar),
      def("get_stack_entry", &get_stack_entry, raw(_1)),
      def("debugstack_annotated", &debugstack_annotated, raw(_1))
    ];
  }
  
  adaptaload("wrap.lua");
  adaptaload_wrapped("stage.lua", argc, argv);
  
  /*adaptaload_wrapped("ui.lua");
  
  if(FLAGS_editor) {
    adaptaload_wrapped("editor.lua");
  } else {
    adaptaload_wrapped("main.lua");
  }*/
  
  if(last_preserved_token.size()) {
    lua_getglobal(L, "generic_wrap");
    lua_getglobal(L, "de_fuckshit");
    lua_pushstring(L, last_preserved_token.c_str());
    int rv = lua_pcall(L, 2, 0, 0);
    if (rv) {
      CHECK(0, "%s", lua_tostring(L, -1));
    }
  }
  
  uilayer = new Layer();
}
void luashutdown() {
  dprintf("lua closing");
  lua_close(L);
  dprintf("lua closed");
  L = NULL;
  
  delete uilayer;
}

void glorp_init(const string &name, const string &fontname, int width, int height, int argc, const char **argv) {
  
  //dprintf("inity");
  // Initialize
  LogToFunction(&log_to_debugstring);
  System::Init();

  setInitFlagFile("glorp/settings");
  initProgram(&argc, const_cast<const char ***>(&argv));
  
  window()->SetTitle(name);
  //window()->SetVSync(true);
  
  {
    GlopWindowSettings gws;
    gws.min_aspect_ratio = (float)width / height;
    gws.min_inverse_aspect_ratio = (float)height / width;
    gws.is_resizable = false;
    ASSERT(window()->Create(width, height, false, gws));
    window()->SetVSync(true);
  }
  
  {
    //Font *font = ShadowFont::Load(fontname.c_str(), 0.05, 0.05);
    Font *font = Font::Load(fontname.c_str());
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
  
  luainit(argc, argv);
  
  {
    KeyList listen_to_shit;
    
    int lasttick = system()->GetTime();
    while(window()->IsCreated()) {
      system()->Think();
      
      {
        PerfStack pb(0.0, 0.0, 0.5);
        
        int thistick = system()->GetTime();
        lua_getglobal(L, "generic_wrap");
        lua_getglobal(L, "UI_Loop");
        lua_pushnumber(L, thistick - lasttick);
        lasttick = thistick;
        int rv = lua_pcall(L, 2, 0, 0);
        if (rv) {
          dprintf("Crash\n");
          dprintf("%s", lua_tostring(L, -1));
          meltdown();
          CHECK(0);
        }
      }
      
      if(input()->IsKeyDownFrame(kKeyF12)) {
        meltdown();
        luashutdown();
        luainit(argc, argv);
      }
      
      {
        PerfStack pb(0, 0.5, 0);
        lua_getglobal(L, "generic_wrap");
        lua_getglobal(L, "gcstep");
        int rv = lua_pcall(L, 1, 0, 0);
        if (rv) {
          dprintf("Crash\n");
          dprintf("%s", lua_tostring(L, -1));
          meltdown();
          CHECK(0);
        }
      }
      
      // siiiigh
      for(int i = 0; i < to_delete_lists.size(); i++) glDeleteLists(to_delete_lists[i], 1);
      for(int i = 0; i < to_delete_shaders.size(); i++) glDeleteShader(to_delete_shaders[i]);
      for(int i = 0; i < to_delete_programs.size(); i++) glDeleteProgram(to_delete_programs[i]);
      to_delete_lists.clear();
      to_delete_shaders.clear();
      to_delete_programs.clear();
      
      if(exiting) {
        window()->Destroy();
      }
    }
  }
  
  meltdown();
  luashutdown();
  
  clean_up_sounds();
  System::ShutDown();
  
  dprintf("exiting");
}

SoundSample *SSLoad(const string &fname_base, float vol) {
  SoundSample *rv;
  rv = SoundSample::Load("data/" + fname_base + ".ogg", false, vol);
  if(rv) return rv;
  rv = SoundSample::Load("data/" + fname_base + ".wav", false, vol);
  if(rv) return rv;
  CHECK(0, fname_base.c_str());
}
