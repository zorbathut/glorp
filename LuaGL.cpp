/*************************************************
*  LuaGL - an OpenGL binding for Lua
*  2003-2004(c) Fabio Guerra, Cleyde Marlyse
*  www.luagl.sourceforge.net
*-------------------------------------------------
*  Description: This file implements the OpenGL
*               binding for Lua 5.0.
*-------------------------------------------------
*  Last Update: 14/07/2004
*  Version: v1.01
*-------------------------------------------------
*  See Copyright Notice in LuaGL.h
*************************************************/

#include <string.h>
#include <stdlib.h>

#include <lua.hpp>
#include <lauxlib.h>

#include "LuaGL.h"
#include "LuaGL_common.h"

#include "debug.h"

#include <assert.h>

/*Accum (op, value) -> none*/
static int gl_accum(lua_State *L)
{
   /* get string parameters */
   GLenum e;

   /* test argument */
   if(!lua_isstring(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.Accum'");

   e = get_gl_enum(L, 1);

   /* test arguments */
   if(e == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.Accum'");

   if(!lua_isnumber(L, 2))
      luaL_error(L, "incorrect argument to function 'gl.Accum'");

   /* call opengl function */
   glAccum(e, (GLfloat)lua_tonumber(L, 2));

   return 0;
}

/*AreTexturesResident (texturesArray) -> residences*/
static int gl_are_textures_resident(lua_State *L)
{
   GLboolean *residences;
   GLuint *textures;

   int i, n;

   /* test argument */
   if(!lua_istable(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.AreTexturesResident'");

   /* get textures array */
   n = get_arrayui(L, 1, &textures);

   residences = (GLboolean *)malloc(n * sizeof(GLboolean));

   /* call opengl function */
   glAreTexturesResident(n, (GLuint *)textures, residences);

   lua_newtable(L);

   /* return residences values */
   for(i = 0; i < n; i++)
      set_field(L, i+1, residences[i]);

   free(textures);
   free(residences);

   return 1;
}

/*ArrayElement (i) -> none*/
static int gl_array_element(lua_State *L)
{
   /* test argument */
   if(!lua_isnumber(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.ArrayElement'");

   /* call opengl function */
   glArrayElement((GLint)lua_tonumber(L, 1));

   return 0;
}

/*Begin (mode) -> none*/
static int gl_begin(lua_State *L)
{
   GLenum e;

   /* test argument */
   if(!lua_isstring(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.Begin'");

   e = get_gl_enum(L, 1);

   /* test argument */
   if(e == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.Begin'");

   /* call opengl function */
   glBegin(e);

   return 0;
}

/*Bitmap (xorig, yorig, ymove, bitmap) -> none*/
static int gl_bitmap(lua_State *L)
{
   int width, height;

   GLubyte *bitmap;

   /* test arguments type */
   if(!( lua_isnumber(L, 1) && lua_isnumber(L, 2) &&
         lua_isnumber(L, 3) && lua_isnumber(L, 4) && lua_istable(L,5) ))
      luaL_error(L, "incorrect argument to function 'gl.Bitmap'");

   if((height = get_array2ubyte(L, 4, &bitmap, &width)) == -1)
      luaL_error(L, "incorrect argument to function 'gl.Bitmap'");

   glBitmap(width, height, (GLfloat)lua_tonumber(L, 1), (GLfloat)lua_tonumber(L, 2),
            (GLfloat)lua_tonumber(L, 3), (GLfloat)lua_tonumber(L, 4), bitmap);
   return 0;
}

/*CallList (list) -> none*/
static int gl_call_list(lua_State *L)
{
   /* test argument */
   if(!lua_isnumber(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.CallList'");

   /* call opengl function */
   glCallList((GLuint)lua_tonumber(L, 1));

   return 0;
}

/*CallLists (listArray) -> none*/
static int gl_call_lists(lua_State *L)
{
   GLsizei n;
   GLfloat *lists;

   /* test argument */
   if(!lua_istable(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.CallLists'");

   /* get array of lists */
   n = get_arrayf(L, 1, &lists);

   /* call opengl function */
   glCallLists(n, GL_FLOAT, lists);

   free(lists);

   return 0;
}

/*ClearAccum (red, green, blue, alpha) -> none*/
static int gl_clear_accum(lua_State *L)
{
   /* test arguments type */
   if(!( lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3) && lua_isnumber(L, 4) ))
      luaL_error(L, "incorrect argument to function 'gl.ClearAccum'");

   /* call opengl function */
   glClearAccum((GLfloat)lua_tonumber(L, 1), (GLfloat)lua_tonumber(L, 2),
                (GLfloat)lua_tonumber(L, 3), (GLfloat)lua_tonumber(L, 4));

   return 0;
}

/*ClearIndex (c) -> none*/
static int gl_clear_index(lua_State *L)
{
   /* test argument type */
   if(!lua_isnumber(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.ClearIndex'");

   /* call opengl function */
   glClearIndex((GLfloat)lua_tonumber(L, 1));

   return 0;
}

/*Color (red, green, blue [, alpha]) -> none
  Color (color) -> none*/
static int gl_color(lua_State *L)
{
   GLdouble *array = 0;

   int index;
   int num_args = lua_gettop(L);

   /* test arguments type */
   if(lua_istable(L, 1))
   {
      num_args = get_arrayd(L, 1, &array);

      /* if more then 4 arguments, ignore the others */
      if(num_args > 4)
         num_args = 4;

      /* call openGL functions */
      switch(num_args)
      {
         case 3:  glColor3dv(array); break;
         case 4:  glColor4dv(array); break;
      }

      if(array)
         free(array);

      return 0;
   }

   /* if more then 4 arguments, ignore the others */
   if(num_args > 4)
      num_args = 4;

   for(index = 0; index < num_args; index++)
   {
      if(!lua_isnumber(L, index + 1))
         luaL_error(L, "incorrect argument to function 'gl.Color'");
   }

   /* call openGL functions */
   switch(num_args)
   {
      case 3:  glColor3d((GLdouble)lua_tonumber(L, 1), (GLdouble)lua_tonumber(L, 2),
                         (GLdouble)lua_tonumber(L, 3));
               break;
      case 4:  glColor4d((GLdouble)lua_tonumber(L, 1), (GLdouble)lua_tonumber(L, 2),
                         (GLdouble)lua_tonumber(L, 3), (GLdouble)lua_tonumber(L, 4));
               break;
   }
   return 0;
}

/*ColorMask (red, green, blue, alpha) -> none*/
static int gl_color_mask(lua_State *L)
{
   /* test arguments type */
   if(!( lua_isboolean(L, 1) && lua_isboolean(L, 2) && lua_isboolean(L, 3) && lua_isboolean(L, 4) ))
      luaL_error(L, "incorrect argument to function 'gl.ColorMask'");

   glColorMask((GLboolean)lua_toboolean(L, 1), (GLboolean)lua_toboolean(L, 2),
               (GLboolean)lua_toboolean(L, 3), (GLboolean)lua_toboolean(L, 4));

   return 0;
}

/*ColorMaterial (face, mode) -> none*/
static int gl_color_material(lua_State *L)
{
   GLenum e1, e2;

   /* test arguments */
   if(!( lua_isstring(L, 1) && lua_isstring(L, 2) ))
      luaL_error(L, "incorrect argument to function 'gl.ColorMaterial'");

   /* get string parameters */
   e1 = get_gl_enum(L, 1);
   e2 = get_gl_enum(L, 2);

   /* test strings */
   if(e1 == ENUM_ERROR || e2 == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.ColorMaterial'");

   /* call opengl function */
   glColorMaterial(e1, e2);

   return 0;
}

/*ColorPointer (colorArray) -> none*/
static int gl_color_pointer(lua_State *L)
{
   GLint size;
   static GLdouble *array = 0;
   if(array)
      free(array);

   /* test arguments type */
   if(!lua_istable(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.ColorPointer'");

   if(lua_isnumber(L, 2))
   {
      size = (GLint)lua_tonumber(L, 2);
      get_arrayd(L, 1, &array);
   }
   else if(get_array2d(L, 1, &array, &size) == -1)
   {
      luaL_error(L, "incorrect argument to function 'gl.ColorPointer'");
      return 0;
   }

   /* call opengl function */
   glColorPointer(size, GL_DOUBLE, 0, array);

   return 0;
}

/*CopyPixels (x, y, width, height, type) -> none*/
static int gl_copy_pixels(lua_State *L)
{
   GLenum e;

   /* test arguments type */
   if(!( lua_isnumber(L, 1) && lua_isnumber(L, 2) &&
         lua_isnumber(L, 3) && lua_isnumber(L, 4) && lua_isstring(L, 5) ))
      luaL_error(L, "incorrect argument to function 'gl.CopyPixels'");

   /* get string parameter */
   e = get_gl_enum(L, 5);

   /* test argument */
   if(e == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.CopyPixels'");

   /* call opengl function */
   glCopyPixels((GLint)  lua_tonumber(L, 1), (GLint)  lua_tonumber(L, 2),
                (GLsizei)lua_tonumber(L, 3), (GLsizei)lua_tonumber(L, 4), (GLenum)e);

   return 0;
}

/*CopyTexImage (level, internalFormat, border, x, y, width[, height]) -> none*/
static int gl_copy_tex_image(lua_State *L)
{
   GLenum internalFormat;

   int num_args = lua_gettop(L);

   /* test arguments type */
   if(!(lua_isnumber(L, 1) && lua_isstring(L, 2) && lua_isnumber(L, 3) &&
        lua_isnumber(L, 4) && lua_isnumber(L, 5) && lua_isnumber(L, 6) ))
      luaL_error(L, "incorrect argument to function 'gl.CopyTexImage'");

   /* get string parameter */
   internalFormat = get_gl_enum(L, 2);

   /* test argument */
   if(internalFormat == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.CopyTexImage'");

   /* call opengl functions */
   if (num_args > 6 && lua_isnumber(L, 7))
   {
      glCopyTexImage2D(GL_TEXTURE_2D, (GLint)lua_tonumber(L, 1), internalFormat,
                       (GLint)lua_tonumber(L, 4), (GLint)lua_tonumber(L, 5),
                       (GLsizei)lua_tonumber(L, 6), (GLsizei)lua_tonumber(L, 7),
                       (GLint)lua_tonumber(L, 3));
   }
   else
   {
      glCopyTexImage1D(GL_TEXTURE_1D, (GLint)lua_tonumber(L, 1), internalFormat,
                       (GLint)lua_tonumber(L, 4), (GLint)lua_tonumber(L, 5),
                       (GLsizei)lua_tonumber(L, 6), (GLint)lua_tonumber(L, 3));
   }
   return 0;
}

/*CopyTexSubImage (level, x, y, xoffset, width[, yoffset, height]) -> none*/
static int gl_copy_tex_sub_image(lua_State *L)
{
   int index;
   int num_args = lua_gettop(L);

   /* test arguments type */
   for(index = 0; index < num_args; index++)
   {
      if(!lua_isnumber(L, index + 1))
         luaL_error(L, "incorrect argument to function 'gl.CopyTexSubImage'");
   }

   /* call opengl funcitions */
   if(num_args >= 7)
   {
      glCopyTexSubImage2D(GL_TEXTURE_2D,
                          (GLint)lua_tonumber(L, 1), (GLint)lua_tonumber(L, 4),
                          (GLint)lua_tonumber(L, 6), (GLint)lua_tonumber(L, 2),
                          (GLint)lua_tonumber(L, 3), (GLint)lua_tonumber(L, 5),
                          (GLint)lua_tonumber(L, 7));
   }
   else
   {
      glCopyTexSubImage1D(GL_TEXTURE_1D,
                          (GLint)lua_tonumber(L, 1), (GLint)lua_tonumber(L, 4),
                          (GLint)lua_tonumber(L, 2), (GLint)lua_tonumber(L, 3),
                          (GLint)lua_tonumber(L, 5));
   }
   return 0;
}

/*DeleteLists (list, range) -> none*/
static int gl_delete_lists(lua_State *L)
{
   /* test arguments type */
   if(!( lua_isnumber(L, 1) && lua_isnumber(L, 2) ))
      luaL_error(L, "incorrect argument to function 'gl.DeleteLists'");

   /* call opengl function */
   glDeleteLists((GLuint)lua_tonumber(L, 1), (GLsizei)lua_tonumber(L, 2));

   return 0;
}

/*DrawBuffer (mode) -> none*/
static int gl_draw_buffer(lua_State *L)
{
   GLenum e;

   /* test argument type */
   if(!lua_isstring(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.DrawBuffer'");

   /* get string parameter */
   e = get_gl_enum(L, 1);

   /* test argument */
   if(e == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.DrawBuffer'");

   /* call opengl function */
   glDrawBuffer(e);

   return 0;
}

/*DrawPixels (width, height, format, pixels) -> none*/
static int gl_draw_pixels(lua_State *L)
{
   GLenum e;
   GLfloat *pixels;

   /* test arguments type */
   if(!(lua_isnumber(L, 1) && lua_isnumber(L, 2) &&
        lua_isstring(L, 3) && lua_istable (L, 4)) )
      luaL_error(L, "incorrect argument to function 'gl.DrawPixels'");

   /* get parameters */
   e = get_gl_enum(L, 3);
   get_arrayf(L, 4, &pixels);

   /* test argument */
   if(e == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.DrawPixels'");

   /* call opengl function */
   glDrawPixels((GLsizei)lua_tonumber(L, 1), (GLsizei)lua_tonumber(L, 2), e, GL_FLOAT, pixels);

   free(pixels);

   return 0;
}

/*EdgeFlag (flag) -> none*/
static int gl_edge_flag(lua_State *L)
{
   GLboolean *flag;

   if(lua_istable(L, 1))/* test argument type */
   {
      /* get argument */
      get_arrayb(L, 1, &flag);

      /* call opengl function */
      glEdgeFlagv((GLboolean *)flag);

      free(flag);
   }
   else if(lua_isboolean(L, 1))/* test argument type */
      /* call opengl function */
      glEdgeFlag((GLboolean)lua_toboolean(L, 1));

   else
      luaL_error(L, "incorrect argument to function 'gl.EdgeFlag'");

   return 0;
}

/*EdgeFlagPointer (flagsArray) -> none*/
static int gl_edge_flag_pointer(lua_State *L)
{
   static GLboolean *flags = 0;
   if(flags)
      free(flags);

   /* test arguments type */
   if(!lua_istable(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.EdgeFlagPointer'");

   /* get argument */
   get_arrayb(L, 1, &flags);

   /* call opengl function */
   glEdgeFlagPointer(0, flags);

   return 0;
}

/*End () -> none*/
static int gl_end(lua_State *L)
{
   glEnd();
   return 0;
}

/*EndList () -> none*/
static int gl_end_list(lua_State *L)
{
   glEndList();
   return 0;
}

/*EvalCoord (u[, v]) -> none
  EvalCoord (coordArray) -> none*/
static int gl_eval_coord(lua_State *L)
{
   GLdouble *array;

   int index;
   int num_args = lua_gettop(L);

   /* test arguments type */
   if(lua_istable(L, 1))
   {
      /* get_array and return no of elements */
      if(get_arrayd(L, 1, &array) == 1)
      {
         glEvalCoord1dv(array);
         return 0;
      }
      else
         glEvalCoord2dv(array);

      free(array);

      return 0;
   }

   /* if more then 2 arguments, ignore the others */
   if(num_args > 2)
      num_args = 2;

   /* test arguments */
   for(index = 0; index < num_args; index++)
      if(!lua_isnumber(L, index + 1))
         luaL_error(L, "incorrect argument to function 'gl.EvalCoord'");

   /* call openGL functions */
   switch(num_args)
   {
      case 1:  glEvalCoord1d((GLdouble)lua_tonumber(L, 1));
               break;
      case 2:  glEvalCoord2d((GLdouble)lua_tonumber(L, 1), (GLdouble)lua_tonumber(L, 2));
               break;
   }
   return 0;
}

/*EvalMesh (mode, i1, i2[,j1, j2]) -> none*/
static int gl_eval_mesh(lua_State *L)
{
   GLenum e;

   int index;
   int num_args = lua_gettop(L);

   /* test arguments type */
   if(!( lua_isstring(L, 1) && num_args > 2))
      luaL_error(L, "incorrect argument to function 'gl.EvalMesh'");

   for(index = 2; index < num_args; index++)
      if(!lua_isnumber(L, index + 1))
         luaL_error(L, "incorrect argument to function 'gl.EvalMesh'");

   /* get string parameter */
   e = get_gl_enum(L, 1);

   /* test argument */
   if(e == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.EvalMesh'");

   /* call opengl function */
   if(num_args < 5)
      glEvalMesh1(e, (GLint)lua_tonumber(L, 2), (GLint)lua_tonumber(L, 3));
   else
      glEvalMesh2(e, (GLint)lua_tonumber(L, 2), (GLint)lua_tonumber(L, 3),
                     (GLint)lua_tonumber(L, 4), (GLint)lua_tonumber(L, 5));

   return 0;
}

/*EvalPoint (i[, j]) -> none*/
static int gl_eval_point(lua_State *L)
{
   int index;
   int num_args = lua_gettop(L);

   /* if more then 2 arguments, ignore the others */
   if(num_args > 2)
      num_args = 2;

   /* test arguments */
   for(index = 0; index < num_args; index++)
      if(!lua_isnumber(L, index + 1))
         luaL_error(L, "incorrect argument to function 'gl.EvalPoint'");

   /* call openGL functions */
   if(num_args == 1)
      glEvalPoint1((GLint)lua_tonumber(L, 1));
   else
      glEvalPoint2((GLint)lua_tonumber(L, 1), (GLint)lua_tonumber(L, 2));

   return 0;
}

/*FeedbackBuffer (size, type) -> dataArray*/
static int gl_feedback_buffer(lua_State *L)
{
   GLfloat *array;
   GLenum e;
   GLsizei size;
   int i;

   if(!( lua_isnumber(L, 1) && lua_isstring(L, 2) ))
      luaL_error(L, "incorrect argument to function 'gl.FeedbackBuffer'");

   /* get parameters */
   size = (GLsizei)lua_tonumber(L, 1);
   e = get_gl_enum(L, 2);

   array = (GLfloat *)malloc(size * sizeof(GLfloat));

   /* test argument */
   if(e == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.FeedbackBuffer'");

   /* call opengl function */
   glFeedbackBuffer (size, e, array);

   lua_newtable(L);

   for(i = 0; i < size; i++)
      set_field(L, i+1, array[i]);

   free(array);

   return 0;
}

/*GenLists (range) -> num*/
static int gl_gen_lists(lua_State *L)
{
   /* test argument type */
   if(!lua_isnumber(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.GenLists'");

   /* call opengl function and push the return value on the stack */
   lua_pushnumber(L, glGenLists((GLsizei)lua_tonumber(L, 1)) );

   return 1;
}

/*GetConst (pname) -> constant string*/
static int gl_get_const(lua_State *L)
{
   int i, size=1;
   GLenum e;
   GLenum *params;
   const char *str;

   /* test argument type */
   if(!lua_isstring(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.GetConst'");

   /* get string parameter */
   e = get_gl_enum(L, 1);

   switch(e)
   {
      case GL_DEPTH_RANGE:
      case GL_MAP1_GRID_DOMAIN:
      case GL_MAP2_GRID_SEGMENTS:
      case GL_MAX_VIEWPORT_DIMS:
      case GL_POINT_SIZE_RANGE:
      case GL_POLYGON_MODE:
         size = 2;
         break;

      case GL_CURRENT_NORMAL:
         size = 3;
         break;

      case GL_ACCUM_CLEAR_VALUE:
      case GL_COLOR_CLEAR_VALUE:
      case GL_COLOR_WRITEMASK:
      case GL_CURRENT_COLOR:
      case GL_CURRENT_RASTER_COLOR:
      case GL_CURRENT_RASTER_POSITION:
      case GL_CURRENT_RASTER_TEXTURE_COORDS:
      case GL_CURRENT_TEXTURE_COORDS:
      case GL_FOG_COLOR:
      case GL_LIGHT_MODEL_AMBIENT:
      case GL_MAP2_GRID_DOMAIN:
      case GL_SCISSOR_BOX:
      case GL_TEXTURE_ENV_COLOR:
      case GL_VIEWPORT:
         size = 4;
         break;

      case GL_MODELVIEW_MATRIX:
      case GL_PROJECTION_MATRIX:
      case GL_TEXTURE_MATRIX:
         size = 16;
         break;
   }
   /* test argument */
   if(e == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.GetConst'");

   params = (GLenum *)malloc(size * sizeof(GLenum));

   /* call opengl function */
   glGetIntegerv(e, (GLint*)params);

   for(i = 0; i < size; i++)
   {
      str = get_str_gl_enum(params[i]);
      lua_pushstring(L, str);
   }

   free(params);

   return size;
}

/*GetArray (pname) -> paramsArray*/
static int gl_get_array(lua_State *L)
{
   int i, size = 1;
   GLenum e;
   GLdouble *params;

   /* test argument type */
   if(!lua_isstring(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.GetArray'");

   /* get string parameter */
   e = get_gl_enum(L, 1);

   /* test argument */
   if(e == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.GetArray'");

   switch(e)
   {
      case GL_DEPTH_RANGE:
      case GL_MAP1_GRID_DOMAIN:
      case GL_MAP2_GRID_SEGMENTS:
      case GL_MAX_VIEWPORT_DIMS:
      case GL_POINT_SIZE_RANGE:
      case GL_POLYGON_MODE:
         size = 2;
         break;

      case GL_CURRENT_NORMAL:
         size = 3;
         break;

      case GL_ACCUM_CLEAR_VALUE:
      case GL_COLOR_CLEAR_VALUE:
      case GL_COLOR_WRITEMASK:
      case GL_CURRENT_COLOR:
      case GL_CURRENT_RASTER_COLOR:
      case GL_CURRENT_RASTER_POSITION:
      case GL_CURRENT_RASTER_TEXTURE_COORDS:
      case GL_CURRENT_TEXTURE_COORDS:
      case GL_FOG_COLOR:
      case GL_LIGHT_MODEL_AMBIENT:
      case GL_MAP2_GRID_DOMAIN:
      case GL_SCISSOR_BOX:
      case GL_TEXTURE_ENV_COLOR:
      case GL_VIEWPORT:
         size = 4;
         break;

      case GL_MODELVIEW_MATRIX:
      case GL_PROJECTION_MATRIX:
      case GL_TEXTURE_MATRIX:
         size = 16;
         break;
   }

   params = (GLdouble *)malloc(size * sizeof(GLdouble));

   /* call opengl function */
   glGetDoublev(e, params);

   lua_newtable(L);

   for(i = 0; i < size; i++)
      set_field(L, i+1, params[i]);

   free(params);

   return 1;
}

/*GetMap (target, query) -> vArray*/
static int gl_get_map(lua_State *L)
{
   int i, size = 1;
   GLenum e1, e2;
   GLdouble *params;
   GLint *order;

   order = (GLint *)malloc(2 * sizeof(GLint));
   order[0] = order[1] = 1;

   /* test arguments type */
   if( !(lua_isstring(L, 1) && lua_isstring(L, 2)) )
      luaL_error(L, "incorrect argument to function 'gl.GetMap'");

   /* get string parameters */
   e1 = get_gl_enum(L, 1);
   e2 = get_gl_enum(L, 2);

   /* test argument */
   if(e1 == ENUM_ERROR || e2 == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.GetMap'");

   switch(e1)
   {
      case GL_MAP1_INDEX:
      case GL_MAP2_INDEX:
      case GL_MAP1_TEXTURE_COORD_1:
      case GL_MAP2_TEXTURE_COORD_1:
         size = 1;
         break;
      case GL_MAP1_TEXTURE_COORD_2:
      case GL_MAP2_TEXTURE_COORD_2:
         size = 2;
         break;
      case GL_MAP1_VERTEX_3:
      case GL_MAP2_VERTEX_3:
      case GL_MAP1_NORMAL:
      case GL_MAP2_NORMAL:
      case GL_MAP1_TEXTURE_COORD_3:
      case GL_MAP2_TEXTURE_COORD_3:
         size = 3;
         break;
      case GL_MAP1_VERTEX_4:
      case GL_MAP2_VERTEX_4:
      case GL_MAP1_COLOR_4:
      case GL_MAP2_COLOR_4:
      case GL_MAP1_TEXTURE_COORD_4:
      case GL_MAP2_TEXTURE_COORD_4:
         size = 4;
         break;
   }

   glGetMapiv(e1, GL_ORDER, order);

   size *= order[0] * order[1];

   params = (GLdouble *)malloc(size * sizeof(GLdouble));

   /* call opengl function */
   glGetMapdv(e1, e2, params);

   lua_newtable(L);

   for(i = 0; i < size; i++)
      set_field(L, i+1, params[i]);

   free(params);

   return 1;
}

/*GetPixelMap (map) -> valuesArray*/
static int gl_get_pixel_map(lua_State *L)
{
   int size;
   int i, s = GL_PIXEL_MAP_R_TO_R_SIZE;
   GLenum e;
   GLfloat *values;

   /* test argument type */
   if(!lua_isstring(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.GetPixelMap'");

   /* get string parameter */
   e = get_gl_enum(L, 1);

   /* test argument */
   if(e == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.GetPixelMap'");

   switch(e)
   {
      case GL_PIXEL_MAP_I_TO_I: s = GL_PIXEL_MAP_I_TO_I_SIZE; break;
      case GL_PIXEL_MAP_S_TO_S: s = GL_PIXEL_MAP_S_TO_S_SIZE; break;
      case GL_PIXEL_MAP_I_TO_R: s = GL_PIXEL_MAP_I_TO_R_SIZE; break;
      case GL_PIXEL_MAP_I_TO_G: s = GL_PIXEL_MAP_I_TO_G_SIZE; break;
      case GL_PIXEL_MAP_I_TO_B: s = GL_PIXEL_MAP_I_TO_B_SIZE; break;
      case GL_PIXEL_MAP_I_TO_A: s = GL_PIXEL_MAP_I_TO_A_SIZE; break;
      case GL_PIXEL_MAP_R_TO_R: s = GL_PIXEL_MAP_R_TO_R_SIZE; break;
      case GL_PIXEL_MAP_G_TO_G: s = GL_PIXEL_MAP_G_TO_G_SIZE; break;
      case GL_PIXEL_MAP_B_TO_B: s = GL_PIXEL_MAP_B_TO_B_SIZE; break;
      case GL_PIXEL_MAP_A_TO_A: s = GL_PIXEL_MAP_A_TO_A_SIZE; break;
   }
   glGetIntegerv(s, &size);

   values = (GLfloat *)malloc(size * sizeof(GLfloat));

   /* call opengl function */
   glGetPixelMapfv(e, values);

   lua_newtable(L);

   for(i = 0; i < size; i++)
      set_field(L, i+1, values[i]);

   free(values);

   return 1;
}

/*GetPolygonStipple () -> maskArray*/
static int gl_get_polygon_stipple(lua_State *L)
{
   int index;
   GLubyte *mask = (GLubyte*)malloc(32*32 * sizeof(GLubyte));

   glGetPolygonStipple(mask);

   lua_newtable(L);

   for(index = 0; index < 1024; index++)
      set_field(L, index+1, mask[index]);

   return 1;
}

/*GetTexGen (coord, pname) -> paramsArray*/
static int gl_get_tex_gen(lua_State *L)
{
   int i;
   GLenum e1, e2;
   GLdouble *params;
   int e3;

   /* test arguments type */
   if( !(lua_isstring(L, 1) && lua_isstring(L, 2)) )
      luaL_error(L, "incorrect argument to function 'gl.GetTexGen'");

   /* get string parameters */
   e1 = get_gl_enum(L, 1);
   e2 = get_gl_enum(L, 2);

   /* test argument */
   if(e1 == ENUM_ERROR || e2 == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.GetTexGen'");
   if(e2 == GL_TEXTURE_GEN_MODE)
   {
      /* call opengl function */
      glGetTexGeniv(e1, e2, &e3);

      lua_pushstring(L, get_str_gl_enum(e3));
   }
   else
   {
      params = (GLdouble *)malloc(4 * sizeof(GLdouble));

      /* call opengl function */
      glGetTexGendv(e1, e2, params);

      lua_newtable(L);

      for(i = 0; i < 4; i++)
         set_field(L, i+1, params[i]);

      free(params);
   }
   return 1;
}

/*GetTexImage (target, level, format) -> pixelsArray*/
static int gl_get_tex_image(lua_State *L)
{
   int i, n=1;
   int width, height, level;
   GLenum target, format;
   GLfloat *pixels;

   /* test arguments type */
   if( !(lua_isstring(L, 1) && lua_isnumber(L, 2) && lua_isstring(L, 3)) )
      luaL_error(L, "incorrect argument to function 'gl.GetTexImage'");

   /* get string parameters */
   target = get_gl_enum(L, 1);
   level = (int)lua_tonumber(L, 2);
   format = get_gl_enum(L, 3);

   /* get width and height of image */
   glGetTexLevelParameteriv(target, level, GL_TEXTURE_WIDTH, &width);
   glGetTexLevelParameteriv(target, level, GL_TEXTURE_HEIGHT, &height);

   switch(format)
   {
      case GL_RED:  case GL_GREEN:  case GL_BLUE:
      case GL_ALPHA: case GL_LUMINANCE: n = 1; break;
      case GL_LUMINANCE_ALPHA:          n = 2; break;
      case GL_RGB:  /*case GL_BGR_EXT:*/    n = 3; break;
      case GL_RGBA: /*case GL_BGRA_EXT:*/   n = 4; break;
      default:
         luaL_error(L, "incorrect string argument to function 'gl.GetTexImage'");
   }

   pixels = (GLfloat *)malloc(n * width * height * sizeof(GLfloat));

   /* call opengl function */
   glGetTexImage(target, level, format, GL_FLOAT, pixels);

   lua_newtable(L);

   for(i = 0; i < n * width * height; i++)
      set_field(L, i+1, pixels[i]);

   free(pixels);

   return 1;
}

/*GetTexLevelParameter (target, level, pname) -> param*/
static int gl_get_tex_level_parameter(lua_State *L)
{
   int level;
   GLenum target, pname;
   GLfloat param;

   /* test arguments type */
   if( !(lua_isstring(L, 1) && lua_isnumber(L, 2) && lua_isstring(L, 3)) )
      luaL_error(L, "incorrect argument to function 'gl.GetTexLevelParameter'");

   /* get parameters */
   target = get_gl_enum(L, 1);
   level = (int)lua_tonumber(L, 2);
   pname = get_gl_enum(L, 3);

   /* call opengl function */
   glGetTexLevelParameterfv(target, level, pname, &param);

   /* return parameter */
   lua_pushnumber(L, param);

   return 1;
}

/*Index (c) -> none*/
static int gl_index(lua_State *L)
{
   GLdouble *c;

   if(lua_istable(L, 1))/* test argument type */
   {
      /* get argument */
      get_arrayd(L, 1, &c);

      /* call opengl function */
      glIndexdv((GLdouble *)c);

      free(c);
   }
   else if(lua_isnumber(L, 1))/* test argument type */
      /* call opengl function */
      glIndexd((GLdouble)lua_tonumber(L, 1));

   else
      luaL_error(L, "incorrect argument to function 'gl.Index'");

   return 0;
}

/*IndexMask (mask) -> none*/
static int gl_index_mask(lua_State *L)
{
   if(lua_type(L,1) == LUA_TSTRING)
      /* call opengl function */
      glIndexMask(str2mask(lua_tostring(L, 1)));

   else if(lua_type(L,1) == LUA_TNUMBER)
      /* call opengl function */
      glIndexMask((GLuint)lua_tonumber(L, 1));

   else
      luaL_error(L, "incorrect argument to function 'gl.IndexMask'");

   return 0;
}

/*IndexPointer (indexArray) -> none*/
static int gl_index_pointer(lua_State *L)
{
   static GLdouble *array = 0;
   if(array)
      free(array);

   /* test arguments type */
   if(!lua_istable(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.IndexPointer'");

   /* get argument */
   get_arrayd(L, 1, &array);

   /* call opengl function */
   glIndexPointer(GL_DOUBLE, 0, array);

   return 0;
}

/*InitNames () -> none*/
static int gl_init_names(lua_State *L)
{
   glInitNames();
   return 0;
}

/*IsEnabled (cap) -> true/false*/
static int gl_is_enabled(lua_State *L)
{
   GLenum e;

   /* test argument type */
   if(!lua_isstring(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.IsEnabled'");

   /* get string parameter */
   e = get_gl_enum(L, 1);

   /* test argument */
   if(e == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.IsEnabled'");

   /* call opengl function */
   lua_pushboolean(L, glIsEnabled(e));

   return 1;
}

/*IsList (list) -> true/false*/
static int gl_is_list(lua_State *L)
{
   /* test argument type */
   if(!lua_isnumber(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.IsList'");

   /* call opengl function and push return value in the lua stack */
   lua_pushboolean(L, glIsList((GLuint)lua_tonumber(L, 1)));

   return 1;
}

/*IsTexture (texture) -> true/false*/
static int gl_is_texture(lua_State *L)
{
   /* test argument type */
   if(!lua_isnumber(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.IsTexture'");

   /* call opengl function and push return value in the lua stack */
   lua_pushboolean(L, glIsTexture((GLuint)lua_tonumber(L, 1)));

   return 1;
}

/*LineStipple (factor, pattern) -> none*/
static int gl_line_stipple(lua_State *L)
{
   /* test arguments type */
   if(!lua_isnumber(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.LineStipple'");

   if(lua_type(L,2) == LUA_TSTRING)
      /* call opengl function */
      glLineStipple((GLint)lua_tonumber(L, 1), (GLushort)str2mask(lua_tostring(L, 2)));

   else if(lua_type(L,2) == LUA_TNUMBER)
      /* call opengl function */
      glLineStipple((GLint)lua_tonumber(L, 1), (GLushort)lua_tonumber(L, 2));

   else
      luaL_error(L, "incorrect argument to function 'gl.LineStipple'");

   return 0;
}

/*ListBase (base) -> none*/
static int gl_list_base(lua_State *L)
{
   /* test argument type */
   if(!lua_isnumber(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.ListBase'");

   /* call opengl function */
   glListBase((GLuint)lua_tonumber(L, 1));

   return 0;
}

/*LoadName (name) -> none*/
static int gl_load_name(lua_State *L)
{
   /* test argument type */
   if(!lua_isnumber(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.LoadName'");

   /* call opengl function */
   glLoadName((GLuint)lua_tonumber(L, 1));

   return 0;
}

/*Map (target, u1, u2, ustride, pointsArray) -> none
  Map (target, u1, u2, ustride, v1, v2, vstride, pointsArray) -> none*/
static int gl_map(lua_State *L)
{
   int size=1;
   GLenum target;
   GLdouble *points;
   GLint uorder, vorder;

   /* test argument */
   if(!( lua_isstring(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3) ))
      luaL_error(L, "incorrect argument to function 'gl.Map'");

   target = get_gl_enum(L, 1);

   switch(target)
   {
      case GL_MAP1_INDEX:
      case GL_MAP2_INDEX:
      case GL_MAP1_TEXTURE_COORD_1:
      case GL_MAP2_TEXTURE_COORD_1:
         size = 1;
         break;
      case GL_MAP1_TEXTURE_COORD_2:
      case GL_MAP2_TEXTURE_COORD_2:
         size = 2;
         break;
      case GL_MAP1_VERTEX_3:
      case GL_MAP2_VERTEX_3:
      case GL_MAP1_NORMAL:
      case GL_MAP2_NORMAL:
      case GL_MAP1_TEXTURE_COORD_3:
      case GL_MAP2_TEXTURE_COORD_3:
         size = 3;
         break;
      case GL_MAP1_VERTEX_4:
      case GL_MAP2_VERTEX_4:
      case GL_MAP1_COLOR_4:
      case GL_MAP2_COLOR_4:
      case GL_MAP1_TEXTURE_COORD_4:
      case GL_MAP2_TEXTURE_COORD_4:
         size = 4;
         break;
   }

   /* test argument */
   if(target == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.Map'");

   /* test number of argument in the array */
   if(lua_gettop(L) < 6)
   {
      if(!lua_istable(L, 4))
         luaL_error(L, "incorrect argument to function 'gl.Map'");

      /* get argument */
      uorder = get_arrayd(L, 4, &points) / size;

      /* call opengl function */
      glMap1d(target, (GLdouble)lua_tonumber(L, 2),
                      (GLdouble)lua_tonumber(L, 3),
                      size, uorder, points);

      free(points);
   }
   else
   {
      if(!( lua_isnumber(L, 4) && lua_isnumber(L, 5) && lua_istable(L, 6) ))
         luaL_error(L, "incorrect argument to function 'gl.Map'");

      /* get argument */
      vorder = get_array2d(L, 6, &points, &uorder);
      uorder /= size;

      /* call opengl function */
      glMap2d(target, (GLdouble)lua_tonumber(L, 2),
                      (GLdouble)lua_tonumber(L, 3),
                      size, uorder,
                      (GLdouble)lua_tonumber(L, 4),
                      (GLdouble)lua_tonumber(L, 5),
                      size * uorder, vorder,
                      points);

      free(points);
   }
   return 0;
}

/*MapGrid (un, u1, u2[, vn, v1, v2]) -> none*/
static int gl_map_grid(lua_State *L)
{
   /* test arguments */
   if(!( lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3) ))
      luaL_error(L, "incorrect argument to function 'gl.MapGrid'");

   /* test number of arguments */
   if(lua_gettop(L) < 6)
   {
      /* call opengl function */
      glMapGrid1d((GLint)lua_tonumber(L, 1),
                  (GLdouble)lua_tonumber(L, 2),
                  (GLdouble)lua_tonumber(L, 3));
   }
   else
   {
      /* test arguments type */
      if(!( lua_isnumber(L, 4) && lua_isnumber(L, 5) && lua_isnumber(L, 6) ))
         luaL_error(L, "incorrect argument to function 'gl.MapGrid'");

      /* call opengl function */
      glMapGrid2d((GLint)lua_tonumber(L, 1),
                  (GLdouble)lua_tonumber(L, 2),
                  (GLdouble)lua_tonumber(L, 3),
                  (GLint)lua_tonumber(L, 4),
                  (GLdouble)lua_tonumber(L, 5),
                  (GLdouble)lua_tonumber(L, 6));
   }
   return 0;
}

/*NewList (list, mode) -> none*/
static int gl_new_list(lua_State *L)
{
   GLenum e;

   /* test arguments type */
   if(!( lua_isnumber(L, 1) && lua_isstring(L, 2) ))
      luaL_error(L, "incorrect argument to function 'gl.NewList'");

   /* get string parameter */
   e = get_gl_enum(L, 2);

   /* test argument */
   if(e == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.NewList'");

   /* call opengl function */
   glNewList((GLint)lua_tonumber(L, 1), e);

   return 0;
}

/*PassThrough (token) -> none*/
static int gl_pass_through(lua_State *L)
{
   /* test argument type */
   if(!lua_isnumber(L, 1))
      luaL_error(L, "incorrect string argument to function 'gl.PassThrough'");

   /* call opengl function */
   glPassThrough((GLfloat)lua_tonumber(L, 1));

   return 0;
}

/*PixelMap (map, valuesArray) -> none*/
static int gl_pixel_map(lua_State *L)
{
   GLenum map;
   GLfloat *values;
   int mapsize;

   /* test arguments */
   if(!( lua_isstring(L, 1) && lua_istable(L, 2) ))
      luaL_error(L, "incorrect argument to function 'gl.PixelMap'");

   /* get values */
   map = get_gl_enum(L, 1);

   /* test argument */
   if(map == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.PixelMap'");

   /* get array of equations */
   mapsize = get_arrayf(L, 2, &values);

   /* call opengl function */
   glPixelMapfv(map, mapsize, values);

   free(values);

   return 0;
}

/*PixelStore (pname, param) -> none*/
static int gl_pixel_store(lua_State *L)
{
   /* get string parameters */
   GLenum e;
   
   /* test argument */
   if(!lua_isstring(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.PixelStore'");

   e = get_gl_enum(L, 1);

   /* test arguments */
   if(e == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.PixelStore'");

   if(lua_isnumber(L, 2))
      /* call opengl function */
      glPixelStoref(e, (GLfloat)lua_tonumber(L, 2));

   else if(lua_isboolean(L,2))
      /* call opengl function */
      glPixelStoref(e, (GLfloat)lua_toboolean(L, 2));

   else
      luaL_error(L, "incorrect argument to function 'gl.PixelStore'");

   return 0;
}

/*PixelTransfer (pname, param) -> none*/
static int gl_pixel_transfer(lua_State *L)
{
   /* get string parameters */
   GLenum e;
   
   /* test argument */
   if(!lua_isstring(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.PixelTransfer'");

   e = get_gl_enum(L, 1);

   /* test arguments */
   if(e == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.PixelTransfer'");

   if(lua_isnumber(L, 2))
      /* call opengl function */
      glPixelTransferf(e, (GLfloat)lua_tonumber(L, 2));

   else if(lua_isboolean(L,2))
      /* call opengl function */
      glPixelTransferf(e, (GLfloat)lua_toboolean(L, 2));

   else
      luaL_error(L, "incorrect argument to function 'gl.PixelTransfer'");

   return 0;
}

/*PixelZoom (xfactor, yfactor) -> none*/
static int gl_pixel_zoom(lua_State *L)
{
   /* test arguments type */
   if(!( lua_isnumber(L, 1) && lua_isnumber(L, 2) ))
      luaL_error(L, "incorrect string argument to function 'gl.PixelZoom'");

   /* call opengl function */
   glPixelZoom((GLfloat)lua_tonumber(L, 1), (GLfloat)lua_tonumber(L, 2));

   return 0;
}

/*PolygonMode (face, mode) -> none*/
static int gl_polygon_mode(lua_State *L)
{
   GLenum e1, e2;

   /* test arguments type */
   if( !(lua_isstring(L, 1) && lua_isstring(L, 2)) )
      luaL_error(L, "incorrect argument to function 'gl.PolygonMode'");

   /* get string parameters */
   e1 = get_gl_enum(L, 1);
   e2 = get_gl_enum(L, 2);

   /* test argument */
   if(e1 == ENUM_ERROR || e2 == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.PolygonMode'");

   /* call opengl function */
   glPolygonMode(e1, e2);

   return 0;
}

/*PolygonStipple (maskArray) -> none*/
static int gl_polygon_stipple(lua_State *L)
{
   GLubyte *array;
   int width, height = 32;

   /* test arguments type */
   if(!lua_istable(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.PolygonStipple'");

   if((height = get_array2ubyte(L, 1, &array, &width)) == -1)
      width = get_arrayubyte(L, 4, &array);

   if(width != 32 && height != 32)
   {
      free(array);
      luaL_error(L, "incorrect argument to function 'gl.PolygonStipple'");
   }

   /* call opengl function */
   glPolygonStipple(array);

   return 0;
}

/*PopAttrib () -> none*/
static int gl_pop_attrib(lua_State *L)
{
   glPopAttrib();
   return 0;
}

/*PopClientAttrib () -> none*/
static int gl_pop_client_attrib(lua_State *L)
{
   glPopClientAttrib();
   return 0;
}

/*PopName () -> none*/
static int gl_pop_name(lua_State *L)
{
   glPopName();
   return 0;
}

/*PrioritizeTextures (texturesArray, prioritiesArray) -> none*/
static int gl_prioritize_textures(lua_State *L)
{
   GLsizei n1, n2;
   GLuint *array1;
   GLclampf *array2;

   /* test arguments type */
   if(!( lua_istable(L, 1) && lua_istable(L, 2) ))
      luaL_error(L, "incorrect argument to function 'gl.PrioritizeTextures'");

   /* get arguments */
   n1 = get_arrayui(L, 1, &array1);
   n2 = get_arrayf(L, 2, &array2);

   /* call opengl function */
   if(n1 > n2) n1 =  n2;

   glPrioritizeTextures(n1, array1, array2);

   free(array1);
   free(array2);

   return 0;
}

/*PushAttrib (mask) -> none*/
static int gl_push_attrib(lua_State *L)
{
   GLbitfield e;

   /* test arguments type */
   if(!lua_isstring(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.PushAttrib'");

   e = get_gl_enum(L, 1);

   /* test arguments */
   if(e == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.PushAttrib'");

   /* call opengl function */
   glPushAttrib(e);

   return 0;
}

/*PushClientAttrib (mask) -> none*/
static int gl_push_client_attrib(lua_State *L)
{
   GLbitfield e;

   /* test arguments type */
   if(!lua_isstring(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.PushClientAttrib'");

   e = get_gl_enum(L, 1);

   /* test arguments */
   if(e == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.PushClientAttrib'");

   /* call opengl function */
   glPushClientAttrib(e);

   return 0;
}

/*PushName (GLuint name) -> none*/
static int gl_push_name(lua_State *L)
{
   /* test arguments type */
   if(!lua_isnumber(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.PushName'");

   /* call opengl function */
   glPushName((GLuint)lua_tonumber(L, 1));

   return 0;
}

/*RasterPos (x, y[, z, w]) -> none
  RasterPos (vArray) -> none*/
static int gl_raster_pos(lua_State *L)
{
   GLdouble *array;

   int index;
   int num_args = lua_gettop(L);

   /* test arguments type */
   if(lua_istable(L, 1))
   {
      num_args = get_arrayd(L, 1, &array);

      /* if more then 4 arguments, ignore the others */
      if(num_args > 4)
         num_args = 4;

      /* call openGL functions */
      switch(num_args)
      {
         case 2:  glRasterPos2dv(array); break;
         case 3:  glRasterPos3dv(array); break;
         case 4:  glRasterPos4dv(array); break;
      }

      free(array);

      return 0;
   }

   /* if more then 4 arguments, ignore the others */
   if(num_args > 4)
      num_args = 4;

   for(index = 0; index < num_args; index++)
   {
      if(!lua_isnumber(L, index + 1))
         luaL_error(L, "incorrect argument to function 'gl.RasterPos'");
   }

   /* call openGL functions */
   switch(num_args)
   {
      case 2:  glRasterPos2d((GLdouble)lua_tonumber(L, 1), (GLdouble)lua_tonumber(L, 2));
               break;
      case 3:  glRasterPos3d((GLdouble)lua_tonumber(L, 1), (GLdouble)lua_tonumber(L, 2),
                             (GLdouble)lua_tonumber(L, 3));
               break;
      case 4:  glRasterPos4d((GLdouble)lua_tonumber(L, 1), (GLdouble)lua_tonumber(L, 2),
                             (GLdouble)lua_tonumber(L, 3), (GLdouble)lua_tonumber(L, 4));
               break;
   }
   return 0;
}

/*ReadBuffer (mode) -> none*/
static int gl_read_buffer(lua_State *L)
{
   GLenum mode;

   /* test argument type */
   if(!lua_isstring(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.ReadBuffer'");

   /* get string parameter */
   mode = get_gl_enum(L, 1);

   /* test argument */
   if(mode == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.ReadBuffer'");

   /* call opengl function */
   glReadBuffer(mode);

   return 0;
}

/*Rect (x1, y1, x2, y2) -> none
  Rect (v1, v2) -> none*/
static int gl_rect(lua_State *L)
{
   GLdouble *v1, *v2;

   /* test argument type */
   if(lua_istable(L, 1) && lua_istable(L, 2))
   {
      /* get parameters */
      get_arrayd(L, 1, &v1);
      get_arrayd(L, 2, &v2);

      /* call opengl function */
      glRectdv(v1, v2);

      free(v1);
      free(v2);
   }
   /* test argument type */
   else if(lua_isnumber(L, 1) && lua_isnumber(L, 2) &&
           lua_isnumber(L, 3) && lua_isnumber(L, 4))
      /* call openGL functions */
      glRectd((GLdouble)lua_tonumber(L, 1), (GLdouble)lua_tonumber(L, 2),
              (GLdouble)lua_tonumber(L, 3), (GLdouble)lua_tonumber(L, 4));

   else
      luaL_error(L, "incorrect argument to function 'gl.Rect'");

   return 0;
}

/*RenderMode (mode) -> none*/
static int gl_render_mode(lua_State *L)
{
   GLenum mode;

   /* test argument type */
   if(!lua_isstring(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.RenderMode'");

   /* get string parameter */
   mode = get_gl_enum(L, 1);

   /* test argument */
   if(mode == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.RenderMode'");

   /* call opengl function */
   glRenderMode(mode);

   return 0;
}

/*SelectBuffer (size) -> SelectArray*/
static int gl_select_buffer(lua_State *L)
{
   int size, i;
   GLuint *buffer;

   /* test arguments type */
   if(!lua_isnumber(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.SelectBuffer'");

   size = (int)lua_tonumber(L, 1);

   buffer = (GLuint *)malloc(size * sizeof(GLuint));

   /* call opengl function */
   glSelectBuffer (size, buffer);

   /* return parameters */
   lua_newtable(L);

   for(i = 0; i < size; i++)
      set_field(L, i+1, buffer[i]);

   free(buffer);

   return 1;
}

/*TexCoord (s[, t, r, q]) -> none
  TexCoord (vArray) -> none*/
static int gl_tex_coord(lua_State *L)
{
   int index;
   int num_args = lua_gettop(L);

   GLdouble *v = 0;

   /* if more then 4 arguments, ignore the others */
   if(num_args > 4)
      num_args = 4;

   /* if have there's no arguments show an error message */
   if(num_args == 0)
      luaL_error(L, "incorrect argument to function 'gl.TexCoord'");

   /* test argument type */
   if(lua_istable(L, 1))
      num_args = get_arrayd(L, 1, &v);

   else
   {
      v = (GLdouble *)malloc(num_args * sizeof(GLdouble));

      /* get arguments */
      for(index = 0; index < num_args; index++)
      {
         /* test arguments type */
         if(!lua_isnumber(L, index + 1))
            luaL_error(L, "incorrect argument to function 'gl.TexCoord'");

         /* get argument */
         v[index] = lua_tonumber(L, index + 1);
      }
   }

   /* call openGL functions */
   switch(num_args)
   {
      case 1:  glTexCoord1dv((GLdouble *)v);  break;
      case 2:  glTexCoord2dv((GLdouble *)v);  break;
      case 3:  glTexCoord3dv((GLdouble *)v);  break;
      case 4:  glTexCoord4dv((GLdouble *)v);  break;
      default: break;
   }

   free(v);

   return 0;
}

/*TexGen (coord, pname, param) -> none
  TexGen (coord, pname, paramsArray) -> none*/
int static gl_tex_gen(lua_State *L)
{
   GLenum e1, e2;
   GLdouble *param;

   /* test arguments type */
   if(!( lua_isstring(L, 1) && lua_isstring(L, 2) ))
      luaL_error(L, "incorrect argument to function 'gl.TexGen'");

   /* get string parameters */
   e1 = get_gl_enum(L, 1);
   e2 = get_gl_enum(L, 2);

   /* test argument */
   if(e1 == ENUM_ERROR || e2 == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.TexGen'");

   if(lua_istable(L, 3))
   {
      get_arrayd(L, 3, &param);

      /* call opengl function */
      glTexGendv(e1, e2, (GLdouble *)param);

      free(param);
   }
   else if(lua_isstring(L, 3))
      /* call opengl function */
      glTexGeni(e1, e2, get_gl_enum(L, 3));

   else
      luaL_error(L, "incorrect argument to function 'gl.TexGen'");
   return 0;
}

/*TexParameter (target, pname, param) -> none
  TexParameter (target, pname, paramsArray) -> none*/
static int gl_tex_parameter(lua_State *L)
{
   GLenum e1, e2;
   GLfloat *param;

   /* test arguments type */
   if(! (lua_isstring(L, 1) && lua_isstring(L, 2) ))
      luaL_error(L, "incorrect argument to function 'gl.TexParameter'");

   /* get string parameters */
   e1 = get_gl_enum(L, 1);
   e2 = get_gl_enum(L, 2);

   /* test argument */
   if(e1 == ENUM_ERROR || e2 == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.TexParameter'");

   if(lua_istable(L, 3))
   {
      get_arrayf(L, 3, &param);

      /* call opengl function */
      glTexParameterfv(e1, e2, (GLfloat *)param);

      free(param);
   }
   else if(lua_isnumber(L, 3))
   {
      /* call opengl function */
      glTexParameterf(e1, e2, (GLfloat)lua_tonumber(L, 3));
   }
   else if(lua_isstring(L, 3))
   {
      /* call opengl function */
      glTexParameteri(e1, e2, get_gl_enum(L, 3));
   }
   else
      luaL_error(L, "incorrect argument to function 'gl.TexParameter'");

   return 0;
}

/*Vertex (x, y, [z, w]) -> none
  Vertex (v) -> none*/
static int gl_vertex(lua_State *L)
{
   int index;
   int num_args = lua_gettop(L);

   GLdouble *v;

   /* if have there's no arguments show an error message */
   if(num_args == 0)
      luaL_error(L, "incorrect argument to function 'gl.Vertex'");

   /* test argument type */
   if(lua_istable(L, 1))
      num_args = get_arrayd(L, 1, &v);

   else
   {
      /* test number of arguments */
      if(num_args < 2)
         luaL_error(L, "incorrect argument to function 'gl.Vertex'");

      v = (GLdouble *)malloc(num_args * sizeof(GLdouble));

      /* get arguments */
      for(index = 0; index < num_args; index++)
      {
         /* test arguments type */
         if(!lua_isnumber(L, index + 1))
            luaL_error(L, "incorrect argument to function 'gl.Vertex'");

         /* get argument */
         v[index] = (GLdouble)lua_tonumber(L, index + 1);
      }
   }

   /* if more then 4 arguments, ignore the others */
   if(num_args > 4)
      num_args = 4;

   /* call openGL functions */
   switch(num_args)
   {
      case 2:  glVertex2dv((GLdouble *)v);  break;
      case 3:  glVertex3dv((GLdouble *)v);  break;
      case 4:  glVertex4dv((GLdouble *)v);  break;
   }

   free(v);

   return 0;
}


static const gl_str_value gl_str[] = {
  MACRIX(TRUE),
  MACRIX(FALSE),
  MACRIX(VERSION_1_1),
  MACRIX(ACCUM),
  MACRIX(LOAD),
  MACRIX(RETURN),
  MACRIX(MULT),
  MACRIX(ADD),
  MACRIX(NEVER),
  MACRIX(LESS),
  MACRIX(EQUAL),
  MACRIX(LEQUAL),
  MACRIX(GREATER),
  MACRIX(NOTEQUAL),
  MACRIX(GEQUAL),
  MACRIX(ALWAYS),
  MACRIX(POINTS),
  MACRIX(LINES),
  MACRIX(LINE_LOOP),
  MACRIX(LINE_STRIP),
  MACRIX(TRIANGLES),
  MACRIX(TRIANGLE_STRIP),
  MACRIX(TRIANGLE_FAN),
  MACRIX(QUADS),
  MACRIX(QUAD_STRIP),
  MACRIX(POLYGON),
  MACRIX(ZERO),
  MACRIX(ONE),
  MACRIX(SRC_COLOR),
  MACRIX(ONE_MINUS_SRC_COLOR),
  MACRIX(SRC_ALPHA),
  MACRIX(ONE_MINUS_SRC_ALPHA),
  MACRIX(DST_ALPHA),
  MACRIX(ONE_MINUS_DST_ALPHA),
  MACRIX(DST_COLOR),
  MACRIX(ONE_MINUS_DST_COLOR),
  MACRIX(SRC_ALPHA_SATURATE),
  MACRIX(CLIP_PLANE0),
  MACRIX(CLIP_PLANE1),
  MACRIX(CLIP_PLANE2),
  MACRIX(CLIP_PLANE3),
  MACRIX(CLIP_PLANE4),
  MACRIX(CLIP_PLANE5),
  MACRIX(BYTE),
  MACRIX(UNSIGNED_BYTE),
  MACRIX(SHORT),
  MACRIX(UNSIGNED_SHORT),
  MACRIX(INT),
  MACRIX(UNSIGNED_INT),
  MACRIX(FLOAT),
  MACRIX(2_BYTES),
  MACRIX(3_BYTES),
  MACRIX(4_BYTES),
  MACRIX(DOUBLE),
  MACRIX(NONE),
  MACRIX(FRONT_LEFT),
  MACRIX(FRONT_RIGHT),
  MACRIX(BACK_LEFT),
  MACRIX(BACK_RIGHT),
  MACRIX(FRONT),
  MACRIX(BACK),
  MACRIX(LEFT),
  MACRIX(RIGHT),
  MACRIX(FRONT_AND_BACK),
  MACRIX(AUX0),
  MACRIX(AUX1),
  MACRIX(AUX2),
  MACRIX(AUX3),
  MACRIX(NO_ERROR),
  MACRIX(INVALID_ENUM),
  MACRIX(INVALID_VALUE),
  MACRIX(INVALID_OPERATION),
  MACRIX(STACK_OVERFLOW),
  MACRIX(STACK_UNDERFLOW),
  MACRIX(OUT_OF_MEMORY),
  MACRIX(2D),
  MACRIX(3D),
  MACRIX(3D_COLOR),
  MACRIX(3D_COLOR_TEXTURE),
  MACRIX(4D_COLOR_TEXTURE),
  MACRIX(PASS_THROUGH_TOKEN),
  MACRIX(POINT_TOKEN),
  MACRIX(LINE_TOKEN),
  MACRIX(POLYGON_TOKEN),
  MACRIX(BITMAP_TOKEN),
  MACRIX(DRAW_PIXEL_TOKEN),
  MACRIX(COPY_PIXEL_TOKEN),
  MACRIX(LINE_RESET_TOKEN),
  MACRIX(EXP),
  MACRIX(EXP2),
  MACRIX(CW),
  MACRIX(CCW),
  MACRIX(COEFF),
  MACRIX(ORDER),
  MACRIX(DOMAIN),
  MACRIX(CURRENT_COLOR),
  MACRIX(CURRENT_INDEX),
  MACRIX(CURRENT_NORMAL),
  MACRIX(CURRENT_TEXTURE_COORDS),
  MACRIX(CURRENT_RASTER_COLOR),
  MACRIX(CURRENT_RASTER_INDEX),
  MACRIX(CURRENT_RASTER_TEXTURE_COORDS),
  MACRIX(CURRENT_RASTER_POSITION),
  MACRIX(CURRENT_RASTER_POSITION_VALID),
  MACRIX(CURRENT_RASTER_DISTANCE),
  MACRIX(POINT_SMOOTH),
  MACRIX(POINT_SIZE),
  MACRIX(POINT_SIZE_RANGE),
  MACRIX(POINT_SIZE_GRANULARITY),
  MACRIX(LINE_SMOOTH),
  MACRIX(LINE_WIDTH),
  MACRIX(LINE_WIDTH_RANGE),
  MACRIX(LINE_WIDTH_GRANULARITY),
  MACRIX(LINE_STIPPLE),
  MACRIX(LINE_STIPPLE_PATTERN),
  MACRIX(LINE_STIPPLE_REPEAT),
  MACRIX(LIST_MODE),
  MACRIX(MAX_LIST_NESTING),
  MACRIX(LIST_BASE),
  MACRIX(LIST_INDEX),
  MACRIX(POLYGON_MODE),
  MACRIX(POLYGON_SMOOTH),
  MACRIX(POLYGON_STIPPLE),
  MACRIX(EDGE_FLAG),
  MACRIX(CULL_FACE),
  MACRIX(CULL_FACE_MODE),
  MACRIX(FRONT_FACE),
  MACRIX(LIGHTING),
  MACRIX(LIGHT_MODEL_LOCAL_VIEWER),
  MACRIX(LIGHT_MODEL_TWO_SIDE),
  MACRIX(LIGHT_MODEL_AMBIENT),
  MACRIX(SHADE_MODEL),
  MACRIX(COLOR_MATERIAL_FACE),
  MACRIX(COLOR_MATERIAL_PARAMETER),
  MACRIX(COLOR_MATERIAL),
  MACRIX(FOG),
  MACRIX(FOG_INDEX),
  MACRIX(FOG_DENSITY),
  MACRIX(FOG_START),
  MACRIX(FOG_END),
  MACRIX(FOG_MODE),
  MACRIX(FOG_COLOR),
  MACRIX(DEPTH_RANGE),
  MACRIX(DEPTH_TEST),
  MACRIX(DEPTH_WRITEMASK),
  MACRIX(DEPTH_CLEAR_VALUE),
  MACRIX(DEPTH_FUNC),
  MACRIX(ACCUM_CLEAR_VALUE),
  MACRIX(STENCIL_TEST),
  MACRIX(STENCIL_CLEAR_VALUE),
  MACRIX(STENCIL_FUNC),
  MACRIX(STENCIL_VALUE_MASK),
  MACRIX(STENCIL_FAIL),
  MACRIX(STENCIL_PASS_DEPTH_FAIL),
  MACRIX(STENCIL_PASS_DEPTH_PASS),
  MACRIX(STENCIL_REF),
  MACRIX(STENCIL_WRITEMASK),
  MACRIX(MATRIX_MODE),
  MACRIX(NORMALIZE),
  MACRIX(VIEWPORT),
  MACRIX(MODELVIEW_STACK_DEPTH),
  MACRIX(PROJECTION_STACK_DEPTH),
  MACRIX(TEXTURE_STACK_DEPTH),
  MACRIX(MODELVIEW_MATRIX),
  MACRIX(PROJECTION_MATRIX),
  MACRIX(TEXTURE_MATRIX),
  MACRIX(ATTRIB_STACK_DEPTH),
  MACRIX(CLIENT_ATTRIB_STACK_DEPTH),
  MACRIX(ALPHA_TEST),
  MACRIX(ALPHA_TEST_FUNC),
  MACRIX(ALPHA_TEST_REF),
  MACRIX(DITHER),
  MACRIX(BLEND_DST),
  MACRIX(BLEND_SRC),
  MACRIX(BLEND),
  MACRIX(LOGIC_OP_MODE),
  MACRIX(LOGIC_OP),
  MACRIX(INDEX_LOGIC_OP),
  MACRIX(COLOR_LOGIC_OP),
  MACRIX(AUX_BUFFERS),
  MACRIX(DRAW_BUFFER),
  MACRIX(READ_BUFFER),
  MACRIX(SCISSOR_BOX),
  MACRIX(SCISSOR_TEST),
  MACRIX(INDEX_CLEAR_VALUE),
  MACRIX(INDEX_WRITEMASK),
  MACRIX(COLOR_CLEAR_VALUE),
  MACRIX(COLOR_WRITEMASK),
  MACRIX(INDEX_MODE),
  MACRIX(RGBA_MODE),
  MACRIX(DOUBLEBUFFER),
  MACRIX(STEREO),
  MACRIX(RENDER_MODE),
  MACRIX(PERSPECTIVE_CORRECTION_HINT),
  MACRIX(POINT_SMOOTH_HINT),
  MACRIX(LINE_SMOOTH_HINT),
  MACRIX(POLYGON_SMOOTH_HINT),
  MACRIX(FOG_HINT),
  MACRIX(TEXTURE_GEN_S),
  MACRIX(TEXTURE_GEN_T),
  MACRIX(TEXTURE_GEN_R),
  MACRIX(TEXTURE_GEN_Q),
  MACRIX(PIXEL_MAP_I_TO_I),
  MACRIX(PIXEL_MAP_S_TO_S),
  MACRIX(PIXEL_MAP_I_TO_R),
  MACRIX(PIXEL_MAP_I_TO_G),
  MACRIX(PIXEL_MAP_I_TO_B),
  MACRIX(PIXEL_MAP_I_TO_A),
  MACRIX(PIXEL_MAP_R_TO_R),
  MACRIX(PIXEL_MAP_G_TO_G),
  MACRIX(PIXEL_MAP_B_TO_B),
  MACRIX(PIXEL_MAP_A_TO_A),
  MACRIX(PIXEL_MAP_I_TO_I_SIZE),
  MACRIX(PIXEL_MAP_S_TO_S_SIZE),
  MACRIX(PIXEL_MAP_I_TO_R_SIZE),
  MACRIX(PIXEL_MAP_I_TO_G_SIZE),
  MACRIX(PIXEL_MAP_I_TO_B_SIZE),
  MACRIX(PIXEL_MAP_I_TO_A_SIZE),
  MACRIX(PIXEL_MAP_R_TO_R_SIZE),
  MACRIX(PIXEL_MAP_G_TO_G_SIZE),
  MACRIX(PIXEL_MAP_B_TO_B_SIZE),
  MACRIX(PIXEL_MAP_A_TO_A_SIZE),
  MACRIX(UNPACK_SWAP_BYTES),
  MACRIX(UNPACK_LSB_FIRST),
  MACRIX(UNPACK_ROW_LENGTH),
  MACRIX(UNPACK_SKIP_ROWS),
  MACRIX(UNPACK_SKIP_PIXELS),
  MACRIX(UNPACK_ALIGNMENT),
  MACRIX(PACK_SWAP_BYTES),
  MACRIX(PACK_LSB_FIRST),
  MACRIX(PACK_ROW_LENGTH),
  MACRIX(PACK_SKIP_ROWS),
  MACRIX(PACK_SKIP_PIXELS),
  MACRIX(PACK_ALIGNMENT),
  MACRIX(MAP_COLOR),
  MACRIX(MAP_STENCIL),
  MACRIX(INDEX_SHIFT),
  MACRIX(INDEX_OFFSET),
  MACRIX(RED_SCALE),
  MACRIX(RED_BIAS),
  MACRIX(ZOOM_X),
  MACRIX(ZOOM_Y),
  MACRIX(GREEN_SCALE),
  MACRIX(GREEN_BIAS),
  MACRIX(BLUE_SCALE),
  MACRIX(BLUE_BIAS),
  MACRIX(ALPHA_SCALE),
  MACRIX(ALPHA_BIAS),
  MACRIX(DEPTH_SCALE),
  MACRIX(DEPTH_BIAS),
  MACRIX(MAX_EVAL_ORDER),
  MACRIX(MAX_LIGHTS),
  MACRIX(MAX_CLIP_PLANES),
  MACRIX(MAX_TEXTURE_SIZE),
  MACRIX(MAX_PIXEL_MAP_TABLE),
  MACRIX(MAX_ATTRIB_STACK_DEPTH),
  MACRIX(MAX_MODELVIEW_STACK_DEPTH),
  MACRIX(MAX_NAME_STACK_DEPTH),
  MACRIX(MAX_PROJECTION_STACK_DEPTH),
  MACRIX(MAX_TEXTURE_STACK_DEPTH),
  MACRIX(MAX_VIEWPORT_DIMS),
  MACRIX(MAX_CLIENT_ATTRIB_STACK_DEPTH),
  MACRIX(SUBPIXEL_BITS),
  MACRIX(INDEX_BITS),
  MACRIX(RED_BITS),
  MACRIX(GREEN_BITS),
  MACRIX(BLUE_BITS),
  MACRIX(ALPHA_BITS),
  MACRIX(DEPTH_BITS),
  MACRIX(STENCIL_BITS),
  MACRIX(ACCUM_RED_BITS),
  MACRIX(ACCUM_GREEN_BITS),
  MACRIX(ACCUM_BLUE_BITS),
  MACRIX(ACCUM_ALPHA_BITS),
  MACRIX(NAME_STACK_DEPTH),
  MACRIX(AUTO_NORMAL),
  MACRIX(MAP1_COLOR_4),
  MACRIX(MAP1_INDEX),
  MACRIX(MAP1_NORMAL),
  MACRIX(MAP1_TEXTURE_COORD_1),
  MACRIX(MAP1_TEXTURE_COORD_2),
  MACRIX(MAP1_TEXTURE_COORD_3),
  MACRIX(MAP1_TEXTURE_COORD_4),
  MACRIX(MAP1_VERTEX_3),
  MACRIX(MAP1_VERTEX_4),
  MACRIX(MAP2_COLOR_4),
  MACRIX(MAP2_INDEX),
  MACRIX(MAP2_NORMAL),
  MACRIX(MAP2_TEXTURE_COORD_1),
  MACRIX(MAP2_TEXTURE_COORD_2),
  MACRIX(MAP2_TEXTURE_COORD_3),
  MACRIX(MAP2_TEXTURE_COORD_4),
  MACRIX(MAP2_VERTEX_3),
  MACRIX(MAP2_VERTEX_4),
  MACRIX(MAP1_GRID_DOMAIN),
  MACRIX(MAP1_GRID_SEGMENTS),
  MACRIX(MAP2_GRID_DOMAIN),
  MACRIX(MAP2_GRID_SEGMENTS),
  MACRIX(TEXTURE_1D),
  MACRIX(TEXTURE_2D),
  MACRIX(FEEDBACK_BUFFER_POINTER),
  MACRIX(FEEDBACK_BUFFER_SIZE),
  MACRIX(FEEDBACK_BUFFER_TYPE),
  MACRIX(SELECTION_BUFFER_POINTER),
  MACRIX(SELECTION_BUFFER_SIZE),
  MACRIX(TEXTURE_WIDTH),
  MACRIX(TEXTURE_HEIGHT),
  MACRIX(TEXTURE_COMPONENTS),
  MACRIX(TEXTURE_INTERNAL_FORMAT),
  MACRIX(TEXTURE_BORDER_COLOR),
  MACRIX(TEXTURE_BORDER),
  MACRIX(DONT_CARE),
  MACRIX(FASTEST),
  MACRIX(NICEST),
  MACRIX(LIGHT0),
  MACRIX(LIGHT1),
  MACRIX(LIGHT2),
  MACRIX(LIGHT3),
  MACRIX(LIGHT4),
  MACRIX(LIGHT5),
  MACRIX(LIGHT6),
  MACRIX(LIGHT7),
  MACRIX(AMBIENT),
  MACRIX(DIFFUSE),
  MACRIX(SPECULAR),
  MACRIX(POSITION),
  MACRIX(SPOT_DIRECTION),
  MACRIX(SPOT_EXPONENT),
  MACRIX(SPOT_CUTOFF),
  MACRIX(CONSTANT_ATTENUATION),
  MACRIX(LINEAR_ATTENUATION),
  MACRIX(QUADRATIC_ATTENUATION),
  MACRIX(COMPILE),
  MACRIX(COMPILE_AND_EXECUTE),
  MACRIX(CLEAR),
  MACRIX(AND),
  MACRIX(AND_REVERSE),
  MACRIX(COPY),
  MACRIX(AND_INVERTED),
  MACRIX(NOOP),
  MACRIX(XOR),
  MACRIX(OR),
  MACRIX(NOR),
  MACRIX(EQUIV),
  MACRIX(INVERT),
  MACRIX(OR_REVERSE),
  MACRIX(COPY_INVERTED),
  MACRIX(OR_INVERTED),
  MACRIX(NAND),
  MACRIX(SET),
  MACRIX(EMISSION),
  MACRIX(SHININESS),
  MACRIX(AMBIENT_AND_DIFFUSE),
  MACRIX(COLOR_INDEXES),
  MACRIX(MODELVIEW),
  MACRIX(PROJECTION),
  MACRIX(TEXTURE),
  MACRIX(COLOR),
  MACRIX(DEPTH),
  MACRIX(STENCIL),
  MACRIX(COLOR_INDEX),
  MACRIX(STENCIL_INDEX),
  MACRIX(DEPTH_COMPONENT),
  MACRIX(RED),
  MACRIX(GREEN),
  MACRIX(BLUE),
  MACRIX(ALPHA),
  MACRIX(RGB),
  MACRIX(RGBA),
  MACRIX(LUMINANCE),
  MACRIX(LUMINANCE_ALPHA),
  MACRIX(BITMAP),
  MACRIX(POINT),
  MACRIX(LINE),
  MACRIX(FILL),
  MACRIX(RENDER),
  MACRIX(FEEDBACK),
  MACRIX(SELECT),
  MACRIX(FLAT),
  MACRIX(SMOOTH),
  MACRIX(KEEP),
  MACRIX(REPLACE),
  MACRIX(INCR),
  MACRIX(DECR),
  MACRIX(VENDOR),
  MACRIX(RENDERER),
  MACRIX(VERSION),
  MACRIX(EXTENSIONS),
  MACRIX(S),
  MACRIX(T),
  MACRIX(R),
  MACRIX(Q),
  MACRIX(MODULATE),
  MACRIX(DECAL),
  MACRIX(TEXTURE_ENV_MODE),
  MACRIX(TEXTURE_ENV_COLOR),
  MACRIX(TEXTURE_ENV),
  MACRIX(EYE_LINEAR),
  MACRIX(OBJECT_LINEAR),
  MACRIX(SPHERE_MAP),
  MACRIX(TEXTURE_GEN_MODE),
  MACRIX(OBJECT_PLANE),
  MACRIX(EYE_PLANE),
  MACRIX(NEAREST),
  MACRIX(LINEAR),
  MACRIX(NEAREST_MIPMAP_NEAREST),
  MACRIX(LINEAR_MIPMAP_NEAREST),
  MACRIX(NEAREST_MIPMAP_LINEAR),
  MACRIX(LINEAR_MIPMAP_LINEAR),
  MACRIX(TEXTURE_MAG_FILTER),
  MACRIX(TEXTURE_MIN_FILTER),
  MACRIX(TEXTURE_WRAP_S),
  MACRIX(TEXTURE_WRAP_T),
  MACRIX(CLAMP),
  MACRIX(REPEAT),
  MACRIX(POLYGON_OFFSET_FACTOR),
  MACRIX(POLYGON_OFFSET_UNITS),
  MACRIX(POLYGON_OFFSET_POINT),
  MACRIX(POLYGON_OFFSET_LINE),
  MACRIX(POLYGON_OFFSET_FILL),
  MACRIX(ALPHA4),
  MACRIX(ALPHA8),
  MACRIX(ALPHA12),
  MACRIX(ALPHA16),
  MACRIX(LUMINANCE4),
  MACRIX(LUMINANCE8),
  MACRIX(LUMINANCE12),
  MACRIX(LUMINANCE16),
  MACRIX(LUMINANCE4_ALPHA4),
  MACRIX(LUMINANCE6_ALPHA2),
  MACRIX(LUMINANCE8_ALPHA8),
  MACRIX(LUMINANCE12_ALPHA4),
  MACRIX(LUMINANCE12_ALPHA12),
  MACRIX(LUMINANCE16_ALPHA16),
  MACRIX(INTENSITY),
  MACRIX(INTENSITY4),
  MACRIX(INTENSITY8),
  MACRIX(INTENSITY12),
  MACRIX(INTENSITY16),
  MACRIX(R3_G3_B2),
  MACRIX(RGB4),
  MACRIX(RGB5),
  MACRIX(RGB8),
  MACRIX(RGB10),
  MACRIX(RGB12),
  MACRIX(RGB16),
  MACRIX(RGBA2),
  MACRIX(RGBA4),
  MACRIX(RGB5_A1),
  MACRIX(RGBA8),
  MACRIX(RGB10_A2),
  MACRIX(RGBA12),
  MACRIX(RGBA16),
  MACRIX(TEXTURE_RED_SIZE),
  MACRIX(TEXTURE_GREEN_SIZE),
  MACRIX(TEXTURE_BLUE_SIZE),
  MACRIX(TEXTURE_ALPHA_SIZE),
  MACRIX(TEXTURE_LUMINANCE_SIZE),
  MACRIX(TEXTURE_INTENSITY_SIZE),
  MACRIX(PROXY_TEXTURE_1D),
  MACRIX(PROXY_TEXTURE_2D),
  MACRIX(TEXTURE_PRIORITY),
  MACRIX(TEXTURE_RESIDENT),
  MACRIX(TEXTURE_BINDING_1D),
  MACRIX(TEXTURE_BINDING_2D),
  MACRIX(VERTEX_ARRAY),
  MACRIX(NORMAL_ARRAY),
  MACRIX(COLOR_ARRAY),
  MACRIX(INDEX_ARRAY),
  MACRIX(TEXTURE_COORD_ARRAY),
  MACRIX(EDGE_FLAG_ARRAY),
  MACRIX(VERTEX_ARRAY_SIZE),
  MACRIX(VERTEX_ARRAY_TYPE),
  MACRIX(VERTEX_ARRAY_STRIDE),
  MACRIX(NORMAL_ARRAY_TYPE),
  MACRIX(NORMAL_ARRAY_STRIDE),
  MACRIX(COLOR_ARRAY_SIZE),
  MACRIX(COLOR_ARRAY_TYPE),
  MACRIX(COLOR_ARRAY_STRIDE),
  MACRIX(INDEX_ARRAY_TYPE),
  MACRIX(INDEX_ARRAY_STRIDE),
  MACRIX(TEXTURE_COORD_ARRAY_SIZE),
  MACRIX(TEXTURE_COORD_ARRAY_TYPE),
  MACRIX(TEXTURE_COORD_ARRAY_STRIDE),
  MACRIX(EDGE_FLAG_ARRAY_STRIDE),
  MACRIX(VERTEX_ARRAY_POINTER),
  MACRIX(NORMAL_ARRAY_POINTER),
  MACRIX(COLOR_ARRAY_POINTER),
  MACRIX(INDEX_ARRAY_POINTER),
  MACRIX(TEXTURE_COORD_ARRAY_POINTER),
  MACRIX(EDGE_FLAG_ARRAY_POINTER),
  MACRIX(V2F),
  MACRIX(V3F),
  MACRIX(C4UB_V2F),
  MACRIX(C4UB_V3F),
  MACRIX(C3F_V3F),
  MACRIX(N3F_V3F),
  MACRIX(C4F_N3F_V3F),
  MACRIX(T2F_V3F),
  MACRIX(T4F_V4F),
  MACRIX(T2F_C4UB_V3F),
  MACRIX(T2F_C3F_V3F),
  MACRIX(T2F_N3F_V3F),
  MACRIX(T2F_C4F_N3F_V3F),
  MACRIX(T4F_C4F_N3F_V4F),
  MACRIX(EXT_vertex_array),
  MACRIX(EXT_bgra),
  MACRIX(EXT_paletted_texture),
  //MACRIX(WIN_swap_hint),
  //MACRIX(WIN_draw_range_elements),
  MACRIX(VERTEX_ARRAY_EXT),
  MACRIX(NORMAL_ARRAY_EXT),
  MACRIX(COLOR_ARRAY_EXT),
  MACRIX(INDEX_ARRAY_EXT),
  MACRIX(TEXTURE_COORD_ARRAY_EXT),
  MACRIX(EDGE_FLAG_ARRAY_EXT),
  MACRIX(VERTEX_ARRAY_SIZE_EXT),
  MACRIX(VERTEX_ARRAY_TYPE_EXT),
  MACRIX(VERTEX_ARRAY_STRIDE_EXT),
  MACRIX(VERTEX_ARRAY_COUNT_EXT),
  MACRIX(NORMAL_ARRAY_TYPE_EXT),
  MACRIX(NORMAL_ARRAY_STRIDE_EXT),
  MACRIX(NORMAL_ARRAY_COUNT_EXT),
  MACRIX(COLOR_ARRAY_SIZE_EXT),
  MACRIX(COLOR_ARRAY_TYPE_EXT),
  MACRIX(COLOR_ARRAY_STRIDE_EXT),
  MACRIX(COLOR_ARRAY_COUNT_EXT),
  MACRIX(INDEX_ARRAY_TYPE_EXT),
  MACRIX(INDEX_ARRAY_STRIDE_EXT),
  MACRIX(INDEX_ARRAY_COUNT_EXT),
  MACRIX(TEXTURE_COORD_ARRAY_SIZE_EXT),
  MACRIX(TEXTURE_COORD_ARRAY_TYPE_EXT),
  MACRIX(TEXTURE_COORD_ARRAY_STRIDE_EXT),
  MACRIX(TEXTURE_COORD_ARRAY_COUNT_EXT),
  MACRIX(EDGE_FLAG_ARRAY_STRIDE_EXT),
  MACRIX(EDGE_FLAG_ARRAY_COUNT_EXT),
  MACRIX(VERTEX_ARRAY_POINTER_EXT),
  MACRIX(NORMAL_ARRAY_POINTER_EXT),
  MACRIX(COLOR_ARRAY_POINTER_EXT),
  MACRIX(INDEX_ARRAY_POINTER_EXT),
  MACRIX(TEXTURE_COORD_ARRAY_POINTER_EXT),
  MACRIX(EDGE_FLAG_ARRAY_POINTER_EXT),
  MACRIX(BGR_EXT),
  MACRIX(BGRA_EXT),
  //MACRIX(COLOR_TABLE_FORMAT_EXT),
  //MACRIX(COLOR_TABLE_WIDTH_EXT),
  //MACRIX(COLOR_TABLE_RED_SIZE_EXT),
  //MACRIX(COLOR_TABLE_GREEN_SIZE_EXT),
  //MACRIX(COLOR_TABLE_BLUE_SIZE_EXT),
  //MACRIX(COLOR_TABLE_ALPHA_SIZE_EXT),
  //MACRIX(COLOR_TABLE_LUMINANCE_SIZE_EXT),
  //MACRIX(COLOR_TABLE_INTENSITY_SIZE_EXT),
  MACRIX(COLOR_INDEX1_EXT),
  MACRIX(COLOR_INDEX2_EXT),
  MACRIX(COLOR_INDEX4_EXT),
  MACRIX(COLOR_INDEX8_EXT),
  MACRIX(COLOR_INDEX12_EXT),
  MACRIX(COLOR_INDEX16_EXT),
  //MACRIX(MAX_ELEMENTS_VERTICES_WIN),
  //MACRIX(MAX_ELEMENTS_INDICES_WIN),
  MACRIX(PHONG_WIN),
  MACRIX(PHONG_HINT_WIN),
  MACRIX(FOG_SPECULAR_TEXTURE_WIN),
  MACRIX(CURRENT_BIT),
  MACRIX(POINT_BIT),
  MACRIX(LINE_BIT),
  MACRIX(POLYGON_BIT),
  MACRIX(POLYGON_STIPPLE_BIT),
  MACRIX(PIXEL_MODE_BIT),
  MACRIX(LIGHTING_BIT),
  MACRIX(FOG_BIT),
  MACRIX(DEPTH_BUFFER_BIT),
  MACRIX(ACCUM_BUFFER_BIT),
  MACRIX(STENCIL_BUFFER_BIT),
  MACRIX(VIEWPORT_BIT),
  MACRIX(TRANSFORM_BIT),
  MACRIX(ENABLE_BIT),
  MACRIX(COLOR_BUFFER_BIT),
  MACRIX(HINT_BIT),
  MACRIX(EVAL_BIT),
  MACRIX(LIST_BIT),
  MACRIX(TEXTURE_BIT),
  MACRIX(SCISSOR_BIT),
  MACRIX(ALL_ATTRIB_BITS),
  MACRIX(CLIENT_PIXEL_STORE_BIT),
  MACRIX(CLIENT_VERTEX_ARRAY_BIT),
  MACRIX(CLIENT_ALL_ATTRIB_BITS),
  
  // this is the stuff I added
  MACRIX(VERTEX_SHADER),
  MACRIX(FRAGMENT_SHADER),
  
  MACRIX(SHADER_TYPE),
  MACRIX(DELETE_STATUS),
  MACRIX(COMPILE_STATUS),
  
  MACRIX(LINK_STATUS),
  MACRIX(VALIDATE_STATUS),
  
  MACRIX(DRAW_FRAMEBUFFER),
  MACRIX(READ_FRAMEBUFFER),
  MACRIX(FRAMEBUFFER),
  MACRIX(RENDERBUFFER),
  MACRIX(DEPTH_ATTACHMENT),
  MACRIX(STENCIL_ATTACHMENT),
  MACRIX(DEPTH_STENCIL_ATTACHMENT),
  MACRIX(COLOR_ATTACHMENT0),
  MACRIX(COLOR_ATTACHMENT1),
  MACRIX(COLOR_ATTACHMENT2),
  MACRIX(COLOR_ATTACHMENT3),
  
  MACRIX(FRAMEBUFFER_COMPLETE),
  MACRIX(FRAMEBUFFER_INCOMPLETE_ATTACHMENT),
  MACRIX(DEPTH_COMPONENT16),
  //MACRIX(FRAMEBUFFER_INCOMPLETE_DIMENSIONS),
  //MACRIX(FRAMEBUFFER_INCOMPLETE_MISSING_DIMENSIONS),
  //MACRIX(FRAMEBUFFER_INCOMPLETE_UNSUPPORTED),
   { 0, 0}
};

static const luaL_reg gllib[] = {
  {"Accum", gl_accum},
  {"AreTexturesResident", gl_are_textures_resident},
  {"ArrayElement", gl_array_element},
  {"Begin", gl_begin},
  {"Bitmap", gl_bitmap},
  {"CallList", gl_call_list},
  {"CallLists", gl_call_lists},
  {"ClearAccum", gl_clear_accum},
  {"ClearIndex", gl_clear_index},
  {"Color", gl_color},
  {"ColorMaterial", gl_color_material},
  {"CopyPixels", gl_copy_pixels},
  {"DeleteLists",gl_delete_lists},
  {"DrawBuffer",gl_draw_buffer},
  {"DrawPixels", gl_draw_pixels},
  {"EdgeFlag", gl_edge_flag},
  {"EdgeFlagPointer", gl_edge_flag_pointer},
  {"End", gl_end},
  {"EndList", gl_end_list},
  {"EvalCoord", gl_eval_coord},
  {"EvalMesh", gl_eval_mesh},
  {"EvalPoint", gl_eval_point},
  {"FeedbackBuffer", gl_feedback_buffer},
  {"GenLists", gl_gen_lists},
  {"GetArray", gl_get_array},
  {"GetConst", gl_get_const},
  {"GetMap", gl_get_map},
  {"GetPixelMap", gl_get_pixel_map},
  {"GetPolygonStipple", gl_get_polygon_stipple},
  {"GetTexGen", gl_get_tex_gen},
  {"GetTexImage", gl_get_tex_image},
  {"GetTexLevelParameter", gl_get_tex_level_parameter},
  {"Index", gl_index},
  {"IndexMask", gl_index_mask},
  {"IndexPointer", gl_index_pointer},
  {"InitNames", gl_init_names},
  {"IsList", gl_is_list},
  {"LineStipple", gl_line_stipple},
  {"ListBase", gl_list_base},
  {"LoadName", gl_load_name},
  {"Map", gl_map},
  {"MapGrid", gl_map_grid},
  {"NewList", gl_new_list},
  {"PassThrough", gl_pass_through},
  {"PixelMap", gl_pixel_map},
  {"PixelTransfer", gl_pixel_transfer},
  {"PixelZoom", gl_pixel_zoom},
  {"PolygonMode", gl_polygon_mode},
  {"PolygonStipple", gl_polygon_stipple},
  {"PopAttrib", gl_pop_attrib},
  {"PopClientAttrib", gl_pop_client_attrib},
  {"PopName", gl_pop_name},
  {"PrioritizeTextures", gl_prioritize_textures},
  {"PushClientAttrib", gl_push_client_attrib},
  {"PushName", gl_push_name},
  {"RasterPos", gl_raster_pos},
  {"ReadBuffer", gl_read_buffer},
  {"Rect", gl_rect},
  {"RenderMode", gl_render_mode},
  {"SelectBuffer", gl_select_buffer},
  {"TexCoord", gl_tex_coord},
  {"TexGen", gl_tex_gen},
  {"Vertex", gl_vertex},
  
  {"ColorMask", gl_color_mask},
  {"CopyTexImage", gl_copy_tex_image},
  {"CopyTexSubImage", gl_copy_tex_sub_image},
  {"PixelStore", gl_pixel_store},
  {"PushAttrib", gl_push_attrib},
  {"TexParameter", gl_tex_parameter},

  {NULL, NULL}
};

int luaopen_opengl (lua_State *L) {
  luagl_table(gl_str);
  
  luaopen_opengl_common(L);
  
  luaL_openlib(L, "gl", gllib, 0);
  return 1;
}
