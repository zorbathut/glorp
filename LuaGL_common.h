#ifndef LUAGL_COMMON
#define LUAGL_COMMON

#include <map>
#include <string>

using namespace std;

#ifdef IPHONE
  #include <OpenGLES/ES1/gl.h>
  #include <OpenGLES/ES1/glext.h>
  #define GLdouble GLfloat
  #define GL_DOUBLE GL_FLOAT
#else
  #include "GLee.h"
#endif

#define ENUM_ERROR (GLenum)-2

typedef struct gl_str_value {
  const char *str;
  GLenum value;
} gl_str_value;

#define MACRIX(ite) {#ite, GL_##ite}

void set_field(lua_State *L, unsigned int index, lua_Number value);

GLenum get_enum(const char *str, int n);
GLenum get_gl_enum(lua_State *L, int index);
const char *get_str_gl_enum(GLenum num);

int get_arrayb(lua_State *L, int index, GLboolean **array);
int get_arrayd(lua_State *L, int index, GLdouble **array);
int get_arrayf(lua_State *L, int index, GLfloat **array);
int get_arrayui(lua_State *L, int index, GLuint **array);
int get_arrayus(lua_State *L, int index, GLushort **array);
int get_arrayubyte(lua_State *L, int index, GLubyte **array);
int get_array2ubyte(lua_State *L, int index, GLubyte **array, int *size);
int get_array2d(lua_State *L, int index, GLdouble **array, int *size);
int get_array2f(lua_State *L, int index, GLfloat **array, int *size);

int str2mask(const char *str);
const char *mask2str(int mask);

void luagl_table(const gl_str_value *gl_str);
void luaopen_opengl_common(lua_State *L);

extern map<string, int> luagl_string_to_enum;
extern map<int, string> luagl_enum_to_string;

#endif
