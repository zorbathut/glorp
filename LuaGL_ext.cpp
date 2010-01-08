
#include <string.h>
#include <stdlib.h>

#include <lua.hpp>
#include <lauxlib.h>

#include "LuaGL_ext.h"

static int gl_shader_source(lua_State *L)
{
  if(!( lua_isnumber(L, 1) && lua_isstring(L, 2) ))
    luaL_error(L, "incorrect argument to function 'gl.ShaderSource'");
  
  size_t strlen;
  const char *str = lua_tolstring(L, 2, &strlen);
  
  glShaderSource((GLuint)lua_tonumber(L, 1), 1, &str, (GLint*)&strlen);
  
  return 0;
}
static int gl_compile_shader(lua_State *L)
{
  if(!( lua_isnumber(L, 1)))
    luaL_error(L, "incorrect argument to function 'gl.CompileShader'");
  
  glCompileShader((GLuint)lua_tonumber(L, 1));
  
  return 0;
}
static int gl_attach_shader(lua_State *L)
{
  if(!(lua_isnumber(L, 1) && lua_isnumber(L, 2)))
    luaL_error(L, "incorrect argument to function 'gl.AttachShader'");
  
  glAttachShader((GLuint)lua_tonumber(L, 1), (GLuint)lua_tonumber(L, 2));
  
  return 0;
}
static int gl_link_program(lua_State *L)
{
  if(!( lua_isnumber(L, 1)))
    luaL_error(L, "incorrect argument to function 'gl.LinkProgram'");
  
  glLinkProgram((GLuint)lua_tonumber(L, 1));
  
  return 0;
}
static int gl_use_program(lua_State *L)
{
  if(!( lua_isnumber(L, 1)))
    luaL_error(L, "incorrect argument to function 'gl.UseProgram'");
  
  glUseProgram((GLuint)lua_tonumber(L, 1));
  
  return 0;
}



static const luaL_reg gllib[] = {
  {"ShaderSource", gl_shader_source},
  {"CompileShader", gl_compile_shader},
  {"AttachShader", gl_attach_shader},
  {"LinkProgram", gl_link_program},
  {"UseProgram", gl_use_program},
  {NULL, NULL}
};

int luaopen_opengl_ext (lua_State *L) {
  luaL_openlib(L, "gl", gllib, 0);
  return 1;
}
