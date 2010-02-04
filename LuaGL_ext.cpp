
#include <string.h>
#include <stdlib.h>

#include <vector>

using namespace std;

#include <lua.hpp>
#include <lauxlib.h>

#include "LuaGL_ext.h"
#include "LuaGL_common.h"

#include "GLee.h"

#include "debug.h"

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

static int gl_get_shader_info_log(lua_State *L)
{
  if(!(lua_isnumber(L, 1)))
    luaL_error(L, "incorrect argument to function 'gl.GetShaderInfoLog'");
  GLint len;
  glGetShaderiv((GLuint)lua_tonumber(L, 1), GL_INFO_LOG_LENGTH, &len);
  
  if(len) {
    vector<char> data(len);
    GLsizei len_ignored;
    glGetShaderInfoLog((GLuint)lua_tonumber(L, 1), len, &len_ignored, &data[0]);
    
    lua_pushlstring(L, &data[0], len - 1);
  } else {
    lua_pushstring(L, "");
  }
  return 1;
}
static int gl_get_program_info_log(lua_State *L)
{
  if(!(lua_isnumber(L, 1)))
    luaL_error(L, "incorrect argument to function 'gl.GetProgramInfoLog'");
  
  GLint len;
  glGetProgramiv((GLuint)lua_tonumber(L, 1), GL_INFO_LOG_LENGTH, &len);
  
  vector<char> data(len);
  glGetProgramInfoLog((GLuint)lua_tonumber(L, 1), len, NULL, &data[0]);
  
  lua_pushlstring(L, &data[0], len - 1);
  return 1;
}

static int gl_get_shader(lua_State *L)
{
  GLenum a;
  
  if(!(lua_isnumber(L, 1) && lua_isstring(L, 2)))
    luaL_error(L, "incorrect argument to function 'gl.GetShader'");
  
  /* get values */
  a = (GLenum)get_gl_enum(L, 2);

  /* test arguments */
  if((a == ENUM_ERROR))
    luaL_error(L, "incorrect string argument to function 'gl.GetShader'");
   
  GLint out;
  glGetShaderiv((GLuint)lua_tonumber(L, 1), a, &out);
  
  lua_pushstring(L, get_str_gl_enum(out));
  return 1;
}
static int gl_get_program(lua_State *L)
{
  GLenum a;
  
  if(!(lua_isnumber(L, 1) && lua_isstring(L, 2)))
    luaL_error(L, "incorrect argument to function 'gl.GetProgram'");
  
  /* get values */
  a = (GLenum)get_gl_enum(L, 2);

  /* test arguments */
  if((a == ENUM_ERROR))
    luaL_error(L, "incorrect string argument to function 'gl.GetProgram'");
   
  GLint out;
  glGetProgramiv((GLuint)lua_tonumber(L, 1), a, &out);
  
  lua_pushstring(L, get_str_gl_enum(out));
  return 1;
}

/*Vertex (l, x, [y, z, w]) -> none*/
static int gl_uniform_i(lua_State *L)
{
   int index;
   int num_args = lua_gettop(L);

   GLint *v;

   /* if have there's no arguments show an error message */
   if(num_args < 2)
      luaL_error(L, "incorrect argument to function 'gl.UniformI'");
   if(!(lua_isnumber(L, 1)))
    luaL_error(L, "incorrect argument to function 'gl.UniformI'");

    v = (GLint *)malloc((num_args - 1) * sizeof(GLint));

    /* get arguments */
    for(index = 1; index < num_args; index++)
    {
       /* test arguments type */
       if(!lua_isnumber(L, index + 1))
          luaL_error(L, "incorrect argument to function 'gl.UniformI'");

       /* get argument */
       v[index - 1] = (GLint)lua_tonumber(L, index + 1);
    }
  
   /* call openGL functions */
   switch(min(num_args - 1, 4))
   {
      case 1:  glUniform1iv((GLint)lua_tonumber(L, 1), 1, v);  break;
      case 2:  glUniform2iv((GLint)lua_tonumber(L, 1), 2, v);  break;
      case 3:  glUniform3iv((GLint)lua_tonumber(L, 1), 3, v);  break;
      case 4:  glUniform4iv((GLint)lua_tonumber(L, 1), 4, v);  break;
   }

   free(v);

   return 0;
}
/*Vertex (l, x, [y, z, w]) -> none*/
static int gl_uniform_f(lua_State *L)
{
   int index;
   int num_args = lua_gettop(L);

   GLfloat *v;

   /* if have there's no arguments show an error message */
   if(num_args < 2)
      luaL_error(L, "incorrect argument to function 'gl.UniformF'");
   if(!(lua_isnumber(L, 1)))
    luaL_error(L, "incorrect argument to function 'gl.UniformF'");

    v = (GLfloat *)malloc((num_args - 1) * sizeof(GLfloat));

    /* get arguments */
    for(index = 1; index < num_args; index++)
    {
       /* test arguments type */
       if(!lua_isnumber(L, index + 1))
          luaL_error(L, "incorrect argument to function 'gl.UniformF'");

       /* get argument */
       v[index - 1] = (GLfloat)lua_tonumber(L, index + 1);
    }
  
   /* call openGL functions */
   switch(min(num_args - 1, 4))
   {
      case 1:  glUniform1fv((GLint)lua_tonumber(L, 1), 1, v);  break;
      case 2:  glUniform2fv((GLint)lua_tonumber(L, 1), 1, v);  break;
      case 3:  glUniform3fv((GLint)lua_tonumber(L, 1), 1, v);  break;
      case 4:  glUniform4fv((GLint)lua_tonumber(L, 1), 1, v);  break;
   }

   free(v);

   return 0;
}
static int gl_get_uniform_location(lua_State *L) {
  if(!(lua_isnumber(L, 1) && lua_isstring(L, 2)))
    luaL_error(L, "incorrect argument to function 'gl.GetUniformLocation'");
   
  GLint out = glGetUniformLocation((GLuint)lua_tonumber(L, 1), lua_tostring(L, 2));
  
  lua_pushnumber(L, out);
  return 1;
}

/*Vertex (l, x, [y, z, w]) -> none*/
static int gl_vertex_attrib_f(lua_State *L)
{
   int index;
   int num_args = lua_gettop(L);

   GLfloat *v;

   /* if have there's no arguments show an error message */
   if(num_args < 2)
      luaL_error(L, "incorrect argument to function 'gl.VertexAttribF'");
   if(!(lua_isnumber(L, 1)))
    luaL_error(L, "incorrect argument to function 'gl.VertexAttribF'");

    v = (GLfloat *)malloc((num_args - 1) * sizeof(GLfloat));

    /* get arguments */
    for(index = 1; index < num_args; index++)
    {
       /* test arguments type */
       if(!lua_isnumber(L, index + 1))
          luaL_error(L, "incorrect argument to function 'gl.VertexAttribF'");

       /* get argument */
       v[index - 1] = (GLfloat)lua_tonumber(L, index + 1);
    }
  
   /* call openGL functions */
   switch(min(num_args - 1, 4))
   {
      case 1:  glVertexAttrib1fv((GLint)lua_tonumber(L, 1), v);  break;
      case 2:  glVertexAttrib2fv((GLint)lua_tonumber(L, 1), v);  break;
      case 3:  glVertexAttrib3fv((GLint)lua_tonumber(L, 1), v);  break;
      case 4:  glVertexAttrib4fv((GLint)lua_tonumber(L, 1), v);  break;
   }

   free(v);

   return 0;
}
static int gl_get_attrib_location(lua_State *L) {
  if(!(lua_isnumber(L, 1) && lua_isstring(L, 2)))
    luaL_error(L, "incorrect argument to function 'gl.GetAttribLocation'");
   
  GLint out = glGetAttribLocation((GLuint)lua_tonumber(L, 1), lua_tostring(L, 2));
  
  lua_pushnumber(L, out);
  return 1;
}

static const luaL_reg gllib[] = {
  {"ShaderSource", gl_shader_source},
  {"CompileShader", gl_compile_shader},
  {"AttachShader", gl_attach_shader},
  {"LinkProgram", gl_link_program},
  {"UseProgram", gl_use_program},
  
  {"GetShaderInfoLog", gl_get_shader_info_log},
  {"GetProgramInfoLog", gl_get_program_info_log},
  {"GetShader", gl_get_shader},
  {"GetProgram", gl_get_program},
  
  {"UniformI", gl_uniform_i},
  {"UniformF", gl_uniform_f},
  {"GetUniformLocation", gl_get_uniform_location},
  
  //{"VertexAttribI", gl_vertex_attrib_i},
  {"VertexAttribF", gl_vertex_attrib_f},
  {"GetAttribLocation", gl_get_attrib_location},
  {NULL, NULL}
};

int luaopen_opengl_ext (lua_State *L) {
  luaL_openlib(L, "gl", gllib, 0);
  return 1;
}
