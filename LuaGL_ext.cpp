
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
      luaL_error(L, "incorrect argument to function 'gl.UniformI', not enough args");
   if(!(lua_isnumber(L, 1)))
    luaL_error(L, "incorrect argument to function 'gl.UniformI', first isn't number");

    v = (GLint *)malloc((num_args - 1) * sizeof(GLint));

    /* get arguments */
    for(index = 1; index < num_args; index++)
    {
       /* test arguments type */
       if(!lua_isnumber(L, index + 1))
          luaL_error(L, "incorrect argument to function 'gl.UniformI', N isn't number");

       /* get argument */
       v[index - 1] = (GLint)lua_tonumber(L, index + 1);
    }
  
   /* call openGL functions */
   switch(min(num_args - 1, 4))
   {
      case 1:  glUniform1iv((GLint)lua_tonumber(L, 1), 1, v);  break;
      case 2:  glUniform2iv((GLint)lua_tonumber(L, 1), 1, v);  break;
      case 3:  glUniform3iv((GLint)lua_tonumber(L, 1), 1, v);  break;
      case 4:  glUniform4iv((GLint)lua_tonumber(L, 1), 1, v);  break;
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

static int gl_bind_framebuffer(lua_State *L)
{
  GLenum a;
  
  if(!(lua_isstring(L, 1) && lua_isnumber(L, 2)))
    luaL_error(L, "incorrect argument to function 'gl.BindFramebuffer'");
  
  /* get values */
  a = (GLenum)get_gl_enum(L, 1);

  /* test arguments */
  if((a == ENUM_ERROR))
    luaL_error(L, "incorrect string argument to function 'gl.BindFramebuffer'");
   
  glBindFramebuffer(a, (GLuint)lua_tonumber(L, 2));
  
  return 0;
}
static int gl_bind_renderbuffer(lua_State *L)
{
  GLenum a;
  
  if(!(lua_isstring(L, 1) && lua_isnumber(L, 2)))
    luaL_error(L, "incorrect argument to function 'gl.BindRenderbuffer'");
  
  /* get values */
  a = (GLenum)get_gl_enum(L, 1);

  /* test arguments */
  if((a == ENUM_ERROR))
    luaL_error(L, "incorrect string argument to function 'gl.BindRenderbuffer'");
   
  glBindRenderbuffer(a, (GLuint)lua_tonumber(L, 2));
  
  return 0;
}

static int gl_renderbuffer_storage(lua_State *L)
{
  GLenum a, b;
  
  if(!(lua_isstring(L, 1) && lua_isstring(L, 2) && lua_isnumber(L, 3) && lua_isnumber(L, 4)))
    luaL_error(L, "incorrect argument to function 'gl.RenderbufferStorage'");
  
  /* get values */
  a = (GLenum)get_gl_enum(L, 1);
  b = (GLenum)get_gl_enum(L, 2);

  /* test arguments */
  if((a == ENUM_ERROR) || (b == ENUM_ERROR))
    luaL_error(L, "incorrect string argument to function 'gl.RenderbufferStorage'");
   
  glRenderbufferStorage(a, b, (GLuint)lua_tonumber(L, 3), (GLuint)lua_tonumber(L, 4));
  
  return 0;
}

static int gl_framebuffer_renderbuffer(lua_State *L)
{
  GLenum a, b, c;
  
  if(!(lua_isstring(L, 1) && lua_isstring(L, 2) && lua_isstring(L, 3) && lua_isnumber(L, 4)))
    luaL_error(L, "incorrect argument to function 'gl.FramebufferRenderbuffer'");
  
  /* get values */
  a = (GLenum)get_gl_enum(L, 1);
  b = (GLenum)get_gl_enum(L, 2);
  c = (GLenum)get_gl_enum(L, 3);

  /* test arguments */
  if((a == ENUM_ERROR) || (b == ENUM_ERROR) || (c == ENUM_ERROR))
    luaL_error(L, "incorrect string argument to function 'gl.FramebufferRenderbuffer'");
   
  glFramebufferRenderbuffer(a, b, c, (GLuint)lua_tonumber(L, 4));
  
  return 0;
}

static int gl_framebuffer_texture_2d(lua_State *L)
{
  GLenum a, b, c;
  
  if(!(lua_isstring(L, 1) && lua_isstring(L, 2) && lua_isstring(L, 3) && lua_isnumber(L, 4) && lua_isnumber(L, 5)))
    luaL_error(L, "incorrect argument to function 'gl.FramebufferTexture2D'");
  
  /* get values */
  a = (GLenum)get_gl_enum(L, 1);
  b = (GLenum)get_gl_enum(L, 2);
  c = (GLenum)get_gl_enum(L, 3);

  /* test arguments */
  if((a == ENUM_ERROR) || (b == ENUM_ERROR) || (c == ENUM_ERROR))
    luaL_error(L, "incorrect string argument to function 'gl.FramebufferTexture2D'");
   
  glFramebufferTexture2D(a, b, c, (GLuint)lua_tonumber(L, 4), (GLuint)lua_tonumber(L, 5));
  
  return 0;
}

bool reported = false;
static int gl_check_framebuffer_status(lua_State *L)
{
  GLenum a;
  
  if(!(lua_isstring(L, 1)))
    luaL_error(L, "incorrect argument to function 'gl.CheckFramebufferStatus'");
  
  /* get values */
  a = (GLenum)get_gl_enum(L, 1);

  /* test arguments */
  if((a == ENUM_ERROR))
    luaL_error(L, "incorrect string argument to function 'gl.CheckFramebufferStatus'");
   
  GLuint glo = glCheckFramebufferStatus(a);
  
  lua_pushstring(L, get_str_gl_enum(glo));
  return 1;
}



/*TexImage(level, internalformat, format, pixels) -> none*/
static int gl_tex_image_2d(lua_State *L)
{
  /* test arguments type */
  if(!(
      lua_isstring(L, 1) && // target
      lua_isnumber(L, 2) && // level
      (lua_isstring(L, 3) || lua_isnumber(L, 3)) && // internalformat
      lua_isnumber(L, 4) && // width
      lua_isnumber(L, 5) && // height
      lua_isnumber(L, 6) && // border
      lua_isstring(L, 7) && // format
      lua_isstring(L, 8) // type
  ))
    luaL_error(L, "incorrect argument to function 'gl.TexImage'");

  GLenum target = get_gl_enum(L, 1);
  GLint level = lua_tointeger(L, 2);
  GLint internalFormat;
  if(lua_isnumber(L, 3)) {
    internalFormat = lua_tointeger(L, 3);
  } else {
    internalFormat = get_gl_enum(L, 3);
  }
  GLsizei width = lua_tointeger(L, 4);
  GLsizei height = lua_tointeger(L, 5);
  GLint border = lua_tointeger(L, 6);
  GLenum format = get_gl_enum(L, 7);
  
  if(target == ENUM_ERROR || internalFormat == ENUM_ERROR || format == ENUM_ERROR)
    luaL_error(L, "incorrect string argument to function 'gl.TexImage'");

  static int shift = 0;
  
  int *x = new int[width * height];
  for(int i = 0; i < width * height; i++)
    x[i] = i * 256 + 0x80000080 + shift;
  shift += 50;
  
  //glTexImage2D(target, level, internalFormat, width, height, border, format, type, x);
  dprintf("teximage %d/%d %d\n", width, height, internalFormat);
  glTexImage2D(target, level, internalFormat, width, height, border, format, GL_UNSIGNED_BYTE, x);
  delete [] x;
  
  return 0;
}



static int gl_ARB_fragment_shader(lua_State *L) {
  lua_pushboolean(L, GLEE_ARB_fragment_shader);
  
  return 1;
}

static int gl_ARB_framebuffer_object(lua_State *L) {
  lua_pushboolean(L, GLEE_ARB_framebuffer_object);
  
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
  
  {"BindFramebuffer", gl_bind_framebuffer},
  {"BindRenderbuffer", gl_bind_renderbuffer},
  {"RenderbufferStorage", gl_renderbuffer_storage},
  {"FramebufferRenderbuffer", gl_framebuffer_renderbuffer},
  {"FramebufferTexture2D", gl_framebuffer_texture_2d},
  {"CheckFramebufferStatus", gl_check_framebuffer_status},
  
  {"ARB_fragment_shader", gl_ARB_fragment_shader},
  {"ARB_framebuffer_object", gl_ARB_framebuffer_object},
  
  {"TexImage2D", gl_tex_image_2d},
  {NULL, NULL}
};

int luaopen_opengl_ext (lua_State *L) {
  luaL_openlib(L, "gl", gllib, 0);

  return 1;
}
