
#include "debug.h"

#ifndef IPHONE
  #include "LuaGL.h"
  #include "LuaGL_ext.h"
#else
  #include "LuaGLES.h"
#endif

#include "LuaGL_common.h"


/* set field of a lua table with a number */
void set_field(lua_State *L, unsigned int index, lua_Number value)
{
   lua_pushnumber(L, index);
   lua_pushnumber(L, value);
   lua_settable(L, -3);
}

GLenum get_enum(const char *str, int n)
{
  map<string, int>::iterator itr = luagl_string_to_enum.find(string(str, n));
  if(itr == luagl_string_to_enum.end()) return ENUM_ERROR;
  return itr->second;
}
GLenum get_gl_enum(lua_State *L, int index)
{
   unsigned int i;
   const char *str = lua_tostring(L, index);
   GLenum temp = 0, ret = 0;

   for(i = 0; i < strlen(str); i++)
   {
      if(str[i] == ',')
      {
         temp = get_enum(str, i);
         if(temp != ENUM_ERROR)
            ret |= temp;

         str += i+1;
         i = 0;
      }
   }
   temp = get_enum(str, strlen(str));

   if(temp == ENUM_ERROR)
   {
      if(ret == 0)
         return ENUM_ERROR;
      return ret;
   }

   return ret | temp;
}

const char *get_str_gl_enum(GLenum num)
{
  map<int, string>::iterator itr = luagl_enum_to_string.find(num);
  if(itr == luagl_enum_to_string.end()) return "ENUM_ERROR";
  return itr->second.c_str();
}

/* Gets an array from a lua table, store it in 'array' and returns the no. of elems of the array
   index refers to where the table is in stack. */
int get_arrayb(lua_State *L, int index, GLboolean **array)
{
   int i;
   int n = luaL_getn(L, index);

   *array = (GLboolean *)malloc(n * sizeof(GLboolean));
   for(i = 0; i < n; i++)
   {
      lua_rawgeti(L, index, i + 1);
      (*array)[i] = (GLboolean)lua_toboolean(L, -1);
   }

   return n; /* return the number of valid elements found.*/
}
int get_arrayd(lua_State *L, int index, GLdouble **array)
{
   int i;
   int n = luaL_getn(L, index);

   *array = (GLdouble *)malloc(n * sizeof(GLdouble));

   for(i = 0; i < n; i++)
   {
      lua_rawgeti(L, index, i + 1);
      (*array)[i] = (GLdouble)lua_tonumber(L, -1);
   }

   return n; /* return the number of valid elements found.*/
}
int get_arrayf(lua_State *L, int index, GLfloat **array)
{
   int i;
   int n = luaL_getn(L, index);

   *array = (GLfloat *)malloc(n * sizeof(GLfloat));

   for(i = 0; i < n; i++)
   {
      lua_rawgeti(L, index, i + 1);
      (*array)[i] = (GLfloat)lua_tonumber(L, -1);
   }

   return n; /* return the number of valid elements found.*/
}
int get_arrayui(lua_State *L, int index, GLuint **array)
{
   int i;
   int n = luaL_getn(L, index);

   *array = (GLuint *)malloc(n * sizeof(GLint));

   for(i = 0; i < n; i++)
   {
      lua_rawgeti(L, index, i + 1);
      (*array)[i] = (GLuint)lua_tonumber(L, -1);
   }

   return n; /* return the number of valid elements found.*/
}
int get_arrayus(lua_State *L, int index, GLushort **array)
{
   int i;
   int n = luaL_getn(L, index);

   *array = (GLushort *)malloc(n * sizeof(GLushort));

   for(i = 0; i < n; i++)
   {
      lua_rawgeti(L, index, i + 1);
      (*array)[i] = (GLushort)lua_tonumber(L, -1);
   }

   return n; /* return the number of valid elements found.*/
}
int get_arrayubyte(lua_State *L, int index, GLubyte **array)
{
   int i;
   int n = luaL_getn(L, index);

   *array = (GLubyte *)malloc(n * sizeof(GLubyte));

   for(i = 0; i < n; i++)
   {
      lua_rawgeti(L, index, i + 1);
      (*array)[i] = (GLubyte)lua_tonumber(L, -1);
   }

   return n; /* return the number of valid elements found.*/
}
int get_array2ubyte(lua_State *L, int index, GLubyte **array, int *size)
{
   int i, j;
   int n = luaL_getn(L, index);

   lua_rawgeti(L, index, 1);

   if(!lua_istable(L, -1))
   {
      lua_remove(L, -1);
      return -1;
   }

   *size = luaL_getn(L, -1);

   *array = (GLubyte *)malloc(n * (*size) * sizeof(GLubyte));

   for(i = 0; i < n; i++)
   {
      lua_rawgeti(L, index, i+1);

      if(!lua_istable(L, -1))
         return -1;

      for(j = 0; j < *size; j++)
      {
         lua_rawgeti(L, -1, j + 1);

         (*array)[i*(*size) + j] = (GLubyte)lua_tonumber(L, -1);

         lua_remove(L, -1);
      }
   }

   return n; /* return the number of valid elements found.*/
}

int get_array2d(lua_State *L, int index, GLdouble **array, int *size)
{
   int i, j;
   int n = luaL_getn(L, index);

   lua_rawgeti(L, index, 1);

   if(!lua_istable(L, -1))
   {
      lua_remove(L, -1);
      return -1;
   }

   *size = luaL_getn(L, -1);

   *array = (GLdouble *)malloc(n * (*size) * sizeof(GLdouble));

   for(i = 0; i < n; i++)
   {
      lua_rawgeti(L, index, i+1);

      if(!lua_istable(L, -1))
         return -1;

      for(j = 0; j < *size; j++)
      {
         lua_rawgeti(L, -1, j + 1);

         (*array)[i*(*size) + j] = (GLdouble)lua_tonumber(L, -1);

         lua_remove(L, -1);
      }
   }

   return n; /* return the number of valid elements found.*/
}
int get_array2f(lua_State *L, int index, GLfloat **array, int *size)
{
   int i, j;
   int n = luaL_getn(L, index);

   lua_rawgeti(L, index, 1);

   if(!lua_istable(L, -1))
   {
      lua_remove(L, -1);
      return -1;
   }

   *size = luaL_getn(L, -1);

   *array = (GLfloat *)malloc(n * (*size) * sizeof(GLfloat));

   for(i = 0; i < n; i++)
   {
      lua_rawgeti(L, index, i+1);

      if(!lua_istable(L, -1))
         return -1;

      for(j = 0; j < *size; j++)
      {
         lua_rawgeti(L, -1, j + 1);

         (*array)[i*(*size) + j] = (GLfloat)lua_tonumber(L, -1);

         lua_remove(L, -1);
      }
   }

   return n; /* return the number of valid elements found.*/
}

int str2mask(const char *str)
{
   int i, j;
   int mask = 0;
   int size = strlen(str);
   for(i = 0, j = 0; j < size; i++)
   {
      if(str[i] == '1')
      {
         mask |= (1 << (size-1-j));
         j++;
      }
      else if(str[i] == '0')
         j++;
         
   }
   return mask;
}
const char *mask2str(int mask)
{
   unsigned int i;
   static char str[17];
   for(i = 0; i < 16; i++)
   {
      if(mask & (1 << (15 - i)))
         str[i] = '1';
      else
         str[i] = '0';
   }
   str[i] = 0;
   return str;
}



/*AlphaFunc (func, ref) -> none*/
static int gl_alpha_func(lua_State *L)
{
   /* get string parameters */
   GLenum e;

   /* test argument */
   if(!lua_isstring(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.AlphaFunc'");

   e = get_gl_enum(L, 1);

   /* test arguments */
   if(e == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.AlphaFunc'");

   if(!lua_isnumber(L, 2))
      luaL_error(L, "incorrect argument to function 'gl.AlphaFunc'");

   /* call opengl function */
   glAlphaFunc(e, (GLclampf)lua_tonumber(L, 2));

   return 0;
}

/*BindTexture (target, texture) -> none*/
static int gl_bind_texture(lua_State *L)
{
   GLenum e;

   /* test arguments */
   if(!( lua_isstring(L, 1) && lua_isnumber(L, 2) ))
      luaL_error(L, "incorrect argument to function 'gl.BindTexture'");

   /* get string value */
   e = get_gl_enum(L, 1);

   /* test arguments */
   if(e == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.BindTexture'");

   /* call opengl function */
   glBindTexture(e, (GLuint)lua_tonumber(L, 2));

   return 0;
}

/*BlendFunc (sfactor, dfactor) -> none*/
static int gl_blend_func(lua_State *L)
{
   GLenum a, b;

   /* test arguments */
   if(!(lua_isstring(L, 1) && lua_isstring(L, 2)))
      luaL_error(L, "incorrect argument to function 'gl.BlendFunc'");

   /* get values */
   a = (GLenum)get_gl_enum(L, 1);
   b = (GLenum)get_gl_enum(L, 2);

   /* test arguments */
   if((a == ENUM_ERROR) || (b == ENUM_ERROR))
      luaL_error(L, "incorrect string argument to function 'gl.BlendFunc'");

   /* call opengl function */
   glBlendFunc(a, b);

   return 0;
}

/*Clear (mask) -> none*/
static int gl_clear(lua_State *L)
{
   GLenum e;

   /* test argument type */
   if(!lua_isstring(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.Clear'");

   e = get_gl_enum(L, 1);

   /* test argument */
   if(e == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.Clear'");

   /* call opengl function */
   glClear(e);

   return 0;
}

/*ClearColor (red, green, blue, alpha) -> none*/
static int gl_clear_color(lua_State *L)
{
   /* test arguments type */
   if(!( lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3) && lua_isnumber(L, 4) ))
      luaL_error(L, "incorrect argument to function 'gl.ClearColor'");

   /* call opengl function */
   glClearColor((GLclampf)lua_tonumber(L, 1), (GLclampf)lua_tonumber(L, 2),
                (GLclampf)lua_tonumber(L, 3), (GLclampf)lua_tonumber(L, 4));

   return 0;
}

/*ClearDepth (depth) -> none*/
static int gl_clear_depth(lua_State *L)
{
   /* test argument type */
   if(!lua_isnumber(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.ClearDepth'");

   /* call opengl function */
   #ifdef IPHONE
   glClearDepthf
   #else
   glClearDepth
   #endif
   ((GLclampf)lua_tonumber(L, 1));

   return 0;
}

/*ClearStencil (s) -> none*/
static int gl_clear_stencil(lua_State *L)
{
   /* test argument type */
   if(!lua_isnumber(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.ClearStencil'");

   /* call opengl function */
   glClearStencil((GLint)lua_tonumber(L, 1));

   return 0;
}

/*ClipPlane (plane, equationArray) -> none*/
static int gl_clip_plane(lua_State *L)
{
   GLenum plane;
   GLdouble *equation;

   /* test arguments */
   if(!lua_isstring(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.ClipPlane'");

   if(!lua_istable(L, 2))
      luaL_error(L, "incorrect argument to function 'gl.ClipPlane'");

   /* get values */
   plane = get_gl_enum(L, 1);

   /* test argument */
   if(plane == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.ClipPlane'");

   /* get array of equations */
   get_arrayd(L, 2, &equation);

   /* call opengl function */
   #ifdef IPHONE
   glClipPlanef
   #else
   glClipPlane
   #endif
   (plane, equation);

   free(equation);

   return 0;
}

/*CullFace (mode) -> none*/
static int gl_cull_face(lua_State *L)
{
   GLenum e;

   /* test argument type */
   if(!lua_isstring(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.CullFace'");

   /* get string parameter */
   e = get_gl_enum(L, 1);

   /* test argument */
   if(e == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.CullFace'");

   /* call opengl function */
   glCullFace(e);

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


/*DeleteTextures (texturesArray) -> none*/
static int gl_delete_textures(lua_State *L)
{
   int n;
   GLuint *textures;

   /* test argument type */
   if(!lua_istable(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.DeleteTextures'");

   /* get textures array */
   n = get_arrayui(L, 1, &textures);

   /* call opengl function */
   glDeleteTextures((GLsizei)n, (GLuint *)textures);

   free(textures);

   return 0;
}

/*DepthFunc (func) -> none*/
static int gl_depth_func(lua_State *L)
{
   GLenum e;

   /* test argument type */
   if(!lua_isstring(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.DepthFunc'");

   /* get string parameter */
   e = get_gl_enum(L, 1);

   /* test argument */
   if(e == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.DepthFunc'");

   /* call opengl function */
   glDepthFunc(e);

   return 0;
}

/*DepthMask (flag) -> none*/
static int gl_depth_mask(lua_State *L)
{
   /* test argument type */
   if(!lua_isboolean(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.DepthMask'");

   /* call opengl function */
   glDepthMask((GLboolean)lua_toboolean(L, 1));

   return 0;
}

/*DepthRange (zNear, zFar) -> none*/
static int gl_depth_range(lua_State *L)
{
   /* test arguments type */
   if(!( lua_isnumber(L, 1) && lua_isnumber(L, 2) ))
      luaL_error(L, "incorrect argument to function 'gl.DepthRange'");

   /* call opengl function */
    #ifdef IPHONE
    glDepthRangef
    #else
    glDepthRange
    #endif
   ((GLclampf)lua_tonumber(L, 1), (GLclampf)lua_tonumber(L, 2));

   return 0;
}

/*Disable (cap) -> none*/
static int gl_disable(lua_State *L)
{
   GLenum e;

   /* test argument type */
   if(!lua_isstring(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.Disable'");

   /* get string parameter */
   e = get_gl_enum(L, 1);

   /* test argument */
   if(e == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.Disable'");

   /* call opengl function */
   glDisable(e);

   return 0;
}

/*DisableClientState (array) -> none*/
static int gl_disable_client_state(lua_State *L)
{
   GLenum e;

   /* test argument type */
   if(!lua_isstring(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.DisableClientState'");

   /* get string parameter */
   e = get_gl_enum(L, 1);

   /* test argument */
   if(e == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.DisableClientState'");

   /* call opengl function */
   glDisableClientState(e);

   return 0;
}

/*DrawArrays (mode, first, count) -> none*/
static int gl_draw_arrays(lua_State *L)
{
   GLenum e;

   /* test arguments type */
   if(!(lua_isstring(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3) ))
      luaL_error(L, "incorrect argument to function 'gl.DrawArrays'");

   /* get string parameter */
   e = get_gl_enum(L, 1);

   /* test argument */
   if(e == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.DrawArrays'");

   /* call opengl function */
   glDrawArrays(e, (GLint)lua_tonumber(L, 2), (GLsizei)lua_tonumber(L, 3));

   return 0;
}

/*DrawElements (mode, indicesArray) -> none*/
static int gl_draw_elements(lua_State *L)
{
   int n;
   GLushort *indices;
   GLenum e;

   /* test arguments type */
   if(!( lua_isstring(L, 1) && lua_istable(L, 2) ))
      luaL_error(L, "incorrect argument to function 'gl.DrawElements'");

   /* get parameters */
   e = get_gl_enum(L, 1);
   n = get_arrayus(L, 2, &indices);

   /* test argument */
   if(e == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.DrawElements'");

   /* call opengl function */
   glDrawElements(e, n, GL_UNSIGNED_SHORT, indices);

   free(indices);

   return 0;
}

/*Enable (cap) -> none*/
static int gl_enable(lua_State *L)
{
   GLenum e;

   /* test argument type */
   if(!lua_isstring(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.Enable'");

   /* get string parameter */
   e = get_gl_enum(L, 1);

   /* test argument */
   if(e == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.Enable'");

   /* call opengl function */
   glEnable(e);
   return 0;
}

/*EnableClientState (array) -> none*/
static int gl_enable_client_state(lua_State *L)
{
   GLenum e;

   /* test argument type */
   if(!lua_isstring(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.EnableClientState'");

   /* get string parameter */
   e = get_gl_enum(L, 1);

   /* test argument */
   if(e == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.EnableClientState'");

   /* call opengl function */
   glEnableClientState(e);

   return 0;
}


/*Finish () -> none*/
static int gl_finish(lua_State *L)
{
   glFinish();
   return 0;
}

/*Flush () -> none*/
static int gl_flush(lua_State *L)
{
   glFlush();
   return 0;
}

/*Fog (pname, param) -> none
  Fog (pname, paramsArray) -> none*/
static int gl_fog(lua_State *L)
{
   GLenum e;
   GLfloat *param;

   /* test first argument type */
   if(!lua_isstring(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.Fog'");

   /* get string parameter */
   e = get_gl_enum(L, 1);

   /* test argument */
   if(e == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.Fog'");

   if(lua_istable(L, 2))
   {
      get_arrayf(L, 2, &param);

      /* call opengl function */
      glFogfv(e, (GLfloat*)param);

      free(param);

      return 0;
   }
   /* test second argument */
   else if(lua_isnumber(L, 2))
   {
      /* call opengl function */
      glFogf(e, (GLfloat)lua_tonumber(L, 2));
   }
   else if(lua_isstring(L, 2))
   {
      #ifdef IPHONE
        CHECK(0);
      #else
        /* call opengl function */
        glFogi(e, get_gl_enum(L, 2));
      #endif
   }
   else
      luaL_error(L, "incorrect argument to function 'gl.Fog'");

   return 0;
}

/*FrontFace (mode) -> none*/
static int gl_front_face(lua_State *L)
{
   GLenum e;

   /* test argument type */
   if(!lua_isstring(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.FrontFace'");

   /* get string parameter */
   e = get_gl_enum(L, 1);

   /* test argument */
   if(e == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.FrontFace'");

   /* call opengl function */
   glFrontFace(e);

   return 0;
}

/*Frustum (left, right, bottom, top, zNear, zFar) -> none*/
static int gl_frustum(lua_State *L)
{
   int index;

   /* test arguments type */
   for(index = 0; index < 6; index++)
      if(!lua_isnumber(L, index + 1))
         luaL_error(L, "incorrect argument to function 'gl.Frustum'");

   /* call opengl function */
  #ifdef IPHONE
   glFrustumf((GLdouble)lua_tonumber(L, 1), (GLdouble)lua_tonumber(L, 2),
             (GLdouble)lua_tonumber(L, 3), (GLdouble)lua_tonumber(L, 4),
             (GLdouble)lua_tonumber(L, 5), (GLdouble)lua_tonumber(L, 6));
  #else
    glFrustum((GLdouble)lua_tonumber(L, 1), (GLdouble)lua_tonumber(L, 2),
           (GLdouble)lua_tonumber(L, 3), (GLdouble)lua_tonumber(L, 4),
           (GLdouble)lua_tonumber(L, 5), (GLdouble)lua_tonumber(L, 6));
    #endif

   return 0;
}

/*GenTextures (n) -> texturesArray*/
static int gl_gen_textures(lua_State *L)
{
   int i;
   GLsizei n;
   GLuint *textures;

   /* test argument type */
   if(!lua_isnumber(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.GenTextures'");

   n = (GLsizei)lua_tonumber(L, 1);
   textures = (GLuint *)malloc(n * sizeof(GLuint));

   /* call opengl function */
   glGenTextures(n, (GLuint *)textures);

   lua_newtable(L);

   for(i = 0; i < n; i++)
      set_field(L, i+1, textures[i]);

   free(textures);

   return 1;
}

/*Get (pname) -> params*/
static int gl_get(lua_State *L)
{
   int i, size=1;
   GLenum e;
   GLdouble *params;
   int mask;

   /* test argument type */
   if(!lua_isstring(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.Get'");

   /* get string parameter */
   e = get_gl_enum(L, 1);

   switch(e)
   {
      case GL_STENCIL_VALUE_MASK:
      case GL_STENCIL_WRITEMASK:
      #ifndef IPHONE
      case GL_LINE_STIPPLE_PATTERN:
      case GL_INDEX_WRITEMASK:
      #endif
         /* call opengl function */
         mask = 0;
         glGetIntegerv(e, &mask);
         lua_pushstring(L, mask2str(mask));
         return 1;

      case GL_DEPTH_RANGE:
      case GL_MAX_VIEWPORT_DIMS:
      #ifndef IPHONE
      case GL_MAP1_GRID_DOMAIN:
      case GL_MAP2_GRID_SEGMENTS:
      case GL_POINT_SIZE_RANGE:
      case GL_POLYGON_MODE:
      #endif
         size = 2;
         break;

      case GL_CURRENT_NORMAL:
         size = 3;
         break;

      case GL_COLOR_CLEAR_VALUE:
      case GL_COLOR_WRITEMASK:
      case GL_CURRENT_COLOR:
      case GL_CURRENT_TEXTURE_COORDS:
      case GL_FOG_COLOR:
      case GL_LIGHT_MODEL_AMBIENT:
      case GL_SCISSOR_BOX:
      case GL_TEXTURE_ENV_COLOR:
      case GL_VIEWPORT:
      #ifndef IPHONE
      case GL_ACCUM_CLEAR_VALUE:
      case GL_CURRENT_RASTER_COLOR:
      case GL_CURRENT_RASTER_POSITION:
      case GL_CURRENT_RASTER_TEXTURE_COORDS:
      case GL_MAP2_GRID_DOMAIN:
      #endif
         size = 4;
         break;

      case GL_MODELVIEW_MATRIX:
      case GL_PROJECTION_MATRIX:
      case GL_TEXTURE_MATRIX:
         size = 16;
         break;

      case ENUM_ERROR:
         luaL_error(L, "incorrect string argument to function 'gl.Get'");
         break;
   }
   dprintf("mallocing %d\n", size * sizeof(GLdouble));
   params = (GLdouble *)malloc(size * sizeof(GLdouble));
  params[0] = 1234;
   
   /* call opengl function */
   #ifdef IPHONE
   glGetFloatv(e, params);
   #else
   glGetDoublev(e, params);
   #endif
   
   dprintf("GLGET2 %d %d %f\n", GL_MAX_PROJECTION_STACK_DEPTH, (int)e, params[0]);
   dprintf("%f\n", params[0]);
   dprintf("err %d\n", glGetError());

   for(i = 0; i < size; i++)
      lua_pushnumber(L, params[i]);

   free(params);

   return size;
}


/*GetClipPlane (plane) -> equationArray*/
static int gl_get_clip_plane(lua_State *L)
{
   int i;
   GLenum e;
   GLdouble *equation;

   /* test argument type */
   if(!lua_isstring(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.GetClipPlane'");

   /* get string parameter */
   e = get_gl_enum(L, 1);

   /* test argument */
   if(e == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.GetClipPlane'");

   equation = (GLdouble *)malloc(4 * sizeof(GLdouble));

   /* call opengl function */
   #ifdef IPHONE
   glGetClipPlanef(e, equation);
   #else
   glGetClipPlane(e, equation);
   #endif

   lua_newtable(L);

   for(i = 0; i < 4; i++)
      set_field(L, i+1, equation[i]);

   free(equation);

   return 1;
}

/*GetError () -> error flag*/
static int gl_get_error(lua_State *L)
{
   /* call glGetError function,
      convert returned number to string,
      and push the string on the stack. */
   GLenum error = glGetError();

   if(error == GL_NO_ERROR)
      lua_pushstring(L, "NO_ERROR");
   else
      lua_pushstring(L, get_str_gl_enum(error));

   return 1;
}

/*GetLight (light, pname) -> paramsArray*/
static int gl_get_light(lua_State *L)
{
   int i, size = 1;
   GLenum e1, e2;
   GLfloat *params;

   /* test arguments type */
   if(!( lua_isstring(L, 1) && lua_isstring(L, 2) ))
      luaL_error(L, "incorrect argument to function 'gl.GetLight'");

   /* get string parameters */
   e1 = get_gl_enum(L, 1);
   e2 = get_gl_enum(L, 2);

   /* test argument */
   if(e1 == ENUM_ERROR || e2 == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.GetLight'");

   switch(e2)
   {
      case GL_AMBIENT:
      case GL_DIFFUSE:
      case GL_SPECULAR:
      case GL_POSITION:
         size = 4;
         break;
      case GL_SPOT_DIRECTION :
         size = 3;
         break;
      case GL_SPOT_EXPONENT:
      case GL_SPOT_CUTOFF:
      case GL_CONSTANT_ATTENUATION:
      case GL_LINEAR_ATTENUATION:
      case GL_QUADRATIC_ATTENUATION:
         size = 1;
         break;
   }

   params = (GLfloat *)malloc(size * sizeof(GLfloat));

   /* call opengl function */
   glGetLightfv(e1, e2, params);

   lua_newtable(L);

   for(i = 0; i < size; i++)
      set_field(L, i+1, params[i]);

   free(params);

   return 1;
}


/*GetMaterial (face, pname) -> paramsArray*/
static int gl_get_material(lua_State *L)
{
   int i, size = 1;
   GLenum e1, e2;
   GLfloat *params;

   /* test arguments type */
   if( !(lua_isstring(L, 1) && lua_isstring(L, 2)) )
      luaL_error(L, "incorrect argument to function 'gl.GetMaterial'");

   /* get string parameters */
   e1 = get_gl_enum(L, 1);
   e2 = get_gl_enum(L, 2);

   switch(e2)
   {
      case GL_AMBIENT:
      case GL_DIFFUSE:
      case GL_SPECULAR:
      case GL_EMISSION:
         size = 4;
         break;
      #ifndef IPHONE
      case GL_COLOR_INDEXES:
         size = 3;
         break;
      #endif
      case GL_SHININESS:
         size = 1;
         break;
   }

   /* test argument */
   if(e1 == ENUM_ERROR || e2 == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.GetMaterial'");

   params = (GLfloat *)malloc(size * sizeof(GLfloat));

   /* call opengl function */
   glGetMaterialfv(e1, e2, params);

   lua_newtable(L);

   for(i = 0; i < size; i++)
      set_field(L, i+1, params[i]);

   free(params);

   return 1;
}

/*GetPointer (pname, n) -> valuesArray*/
static int gl_get_pointer(lua_State *L)
{
   int i, n;
   GLenum e;
   GLboolean *flags;
   GLdouble *params;

   /* test argument type */
   if(!( lua_isstring(L, 1) && lua_isnumber(L, 2) ))
      luaL_error(L, "incorrect argument to function 'gl.GetPointer'");

   e = get_gl_enum(L, 1);
   n = (int)lua_tonumber(L, 2);

   /* test argument */
   if(e == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.GetPointer'");

   #ifndef IPHONE
   if(e == GL_EDGE_FLAG_ARRAY_POINTER)
   {
      flags = (GLboolean *)malloc(n * sizeof(GLboolean));

      /* call opengl function */
      glGetPointerv(e, (void **)&flags);

      if(flags == 0)
         return 0;

      lua_newtable(L);

      for(i = 0; i < n ; i++)
         set_field(L, i+1, flags[i]);
   }
   else
  #endif
   {
    
      params = (GLdouble *)malloc(n * sizeof(GLdouble));

      /* call opengl function */
      glGetPointerv(e, (void **)&params);

      if(params == 0)
         return 0;

      lua_newtable(L);

      for(i = 0; i < n ; i++)
         set_field(L, i+1, params[i]);
   }

   return 1;
}


/*GetString (name) -> string*/
static int gl_get_string(lua_State *L)
{
   GLenum e;
   const GLubyte *str;

   /* test argument type */
   if(!lua_isstring(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.GetString'");

   /* get string parameter */
   e = get_gl_enum(L, 1);

   /* test argument */
   if(e == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.GetString'");

   /* call opengl function */
   str = glGetString(e);

   lua_pushstring(L, (const char*)str);

   return 1;
}

/*GetTexEnv (pname) -> paramsArray*/
static int gl_get_tex_env(lua_State *L)
{
   int i;
   GLenum e1;
   GLfloat *params;
   int e2;

   /* test arguments type */
   if(!lua_isstring(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.GetTexEnv'");

   /* get string parameters */
   e1 = get_gl_enum(L, 1);

   /* test argument */
   if(e1 == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.GetTexEnv'");

   if(e1 == GL_TEXTURE_ENV_MODE)
   {
      glGetTexEnviv(GL_TEXTURE_ENV, e1, &e2);

      lua_pushstring(L, get_str_gl_enum(e2));
   }
   else if(e1 == GL_TEXTURE_ENV_COLOR)
   {
      params = (GLfloat *)malloc(4 * sizeof(GLfloat));

      /* call opengl function */
      glGetTexEnvfv(GL_TEXTURE_ENV, e1, params);

      lua_newtable(L);

      for(i = 0; i < 4; i++)
         set_field(L, i+1, params[i]);

      free(params);
   }
   else
   {
      luaL_error(L, "incorrect string argument to function 'gl.GetTexEnv'");
   }
   return 1;
}

/*GetTexParameter (target, pname) -> paramsArray*/
static int gl_get_tex_parameter(lua_State *L)
{
   int i;
   GLenum target, pname;
   GLfloat *params;
   GLfloat param;
   int e;

   /* test arguments type */
   if(! (lua_isstring(L, 1) && lua_isstring(L, 2) ))
      luaL_error(L, "incorrect argument to function 'gl.GetTexParameter'");

   /* get string parameters */
   target = get_gl_enum(L, 1);
   pname = get_gl_enum(L, 2);

   #ifndef IPHONE
   if(pname == GL_TEXTURE_BORDER_COLOR)
   {
      params = (GLfloat *)malloc(4 * sizeof(float));

      /* call opengl function */
      glGetTexParameterfv(target, pname, params);

      /* return parameters */
      lua_newtable(L);

      for(i = 0; i < 4; i++)
         set_field(L, i+1, params[i]);
   }
   else if(pname == GL_TEXTURE_PRIORITY)
   {
      /* call opengl function */
      glGetTexParameterfv(target, pname, &param);

      lua_pushnumber(L, param);
   }
   else
   #endif
   {
      /* call opengl function */
      glGetTexParameteriv(target, pname, &e);

      lua_pushstring(L, get_str_gl_enum(e));
   }
   return 1;
}

/*Hint (target, mode) -> none*/
static int gl_hint(lua_State *L)
{
   GLenum e1, e2;

   /* test arguments type */
   if( !(lua_isstring(L, 1) && lua_isstring(L, 2)) )
      luaL_error(L, "incorrect argument to function 'gl.Hint'");

   e1 = get_gl_enum(L, 1);
   e2 = get_gl_enum(L, 2);

   /* test argument */
   if(e1 == ENUM_ERROR || e2 == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.Hint'");

   /* call opengl function */
   glHint(e1, e2);

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

/*Light (light, pname, param) -> none
  Light (light, pname, paramsArray) -> none*/
static int gl_light(lua_State *L)
{
   GLenum e1, e2;
   GLfloat *params;

   /* test arguments type */
   if(!( lua_isstring(L, 1) && lua_isstring(L, 2) ))
      luaL_error(L, "incorrect argument to function 'gl.Light'");

   /* get string parameters */
   e1 = get_gl_enum(L, 1);
   e2 = get_gl_enum(L, 2);

   /* test argument */
   if(e1 == ENUM_ERROR || e2 == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.Light'");

   /* test argument type */
   if(lua_istable(L, 3))
   {
      /* get argument */
      get_arrayf(L, 3, &params);

      /* call opengl function */
      glLightfv(e1, e2, (GLfloat *)params);

      free(params);
   }
   /* test argument type */
   else if(lua_isnumber(L, 3))
   {
      /* call opengl function */
      glLightf(e1, e2, (GLfloat)lua_tonumber(L, 3));
   }
   else
      luaL_error(L, "incorrect argument to function 'gl.Light'");

   return 0;
}

/*LightModel (pname, param) -> none
  LightModel (pname, paramsArray) -> none*/
static int gl_light_model(lua_State *L)
{
   GLenum e;
   GLfloat *params;

   /* test argument type */
   if(!lua_isstring(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.LightModel'");

   /* get string parameter */
   e = get_gl_enum(L, 1);

   /* test argument */
   if(e == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.LightModel'");

   /* test argument type */
   if(lua_istable(L, 2))
   {
      /* get argument */
      get_arrayf(L, 2, &params);

      /* call opengl function */
      glLightModelfv(e, (GLfloat *)params);

      free(params);
   }
   /* test argument type */
   else if(lua_isnumber(L, 2))
      /* call opengl function */
      glLightModelf(e, (GLfloat)lua_tonumber(L, 2));

   else
      luaL_error(L, "incorrect argument to function 'gl.LightModel'");

   return 0;
}

/*LineWidth (width) -> none*/
static int gl_line_width(lua_State *L)
{
   /* test argument type */
   if(!lua_isnumber(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.LineWidth'");

   /* call opengl function */
   glLineWidth((GLfloat)lua_tonumber(L, 1));

   return 0;
}

/*LoadIdentity () -> none*/
static int gl_load_identity(lua_State *L)
{
   glLoadIdentity();
   return 0;
}

/*LoadMatrix (mArray) -> none*/
static int gl_load_matrix(lua_State *L)
{
   GLdouble *m;

   /* test argument type and the number of arguments in the array, must be 16 values */
   if(!lua_istable(L, 1) || luaL_getn(L, 1) < 16)
      luaL_error(L, "incorrect argument to function 'gl.LoadMatrix'");

   /* get argument */
   get_arrayd(L, 1, &m);

   /* call opengl function */
   #ifdef IPHONE
   glLoadMatrixf(m);
   #else
   glLoadMatrixd(m);
   #endif

   free(m);

   return 0;
}

/*LogicOp (opcode) -> none*/
static int gl_logic_op(lua_State *L)
{
   GLenum opcode;

   /* test argument type */
   if(!lua_isstring(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.LogicOp'");

   /* get string parameter */
   opcode = get_gl_enum(L, 1);

   /* test argument */
   if(opcode == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.LogicOp'");

   /* call opengl function */
   glLogicOp(opcode);

   return 0;
}

/*Material (face, pname, param) -> none*/
static int gl_material(lua_State *L)
{
   GLenum e1, e2;
   GLfloat *params;

   /* test arguments type */
   if( !(lua_isstring(L, 1) && lua_isstring(L, 2)) )
      luaL_error(L, "incorrect argument to function 'gl.Material'");

   /* get string parameters */
   e1 = get_gl_enum(L, 1);
   e2 = get_gl_enum(L, 2);

   /* test argument */
   if(e1 == ENUM_ERROR || e2 == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.Material'");

   /* test argument type */
   if(lua_istable(L, 3))
   {
      /* get argument */
      get_arrayf(L, 3, &params);

      /* call opengl function */
      glMaterialfv(e1, e2, (GLfloat *)params);

      free(params);
   }
   /* test argument type */
   else if(lua_isnumber(L, 3))
   {
      /* call opengl function */
      glMaterialf(e1, e2, (GLfloat)lua_tonumber(L, 3));
   }
   else
      luaL_error(L, "incorrect argument to function 'gl.Material'");

   return 0;
}

/*MatrixMode (mode) -> none*/
static int gl_matrix_mode(lua_State *L)
{
   GLenum mode;

   /* test argument type */
   if(!lua_isstring(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.MatrixMode'");

   /* get string parameter */
   mode = get_gl_enum(L, 1);

   /* test argument */
   if(mode == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.MatrixMode'");

   /* call opengl function */
   glMatrixMode(mode);

   return 0;
}

/*MultMatrix (mArray) -> none*/
static int gl_mult_matrix(lua_State *L)
{
   GLdouble *m;

   /* test argument type and the number of arguments in the array, must be 16 values */
   if(!lua_istable(L, 1) || luaL_getn(L, 1) < 16)
      luaL_error(L, "incorrect argument to function 'gl.MultMatrix'");

   /* get argument */
   get_arrayd(L, 1, &m);

   /* call opengl function */
   #ifdef IPHONE
   glMultMatrixf(m);
   #else
   glMultMatrixd(m);
   #endif

   free(m);

   return 0;
}

/*Normal (nx, ny, nz) -> none
  Normal (nArray) -> none*/
static int gl_normal(lua_State *L)
{
   GLdouble *array;

   int num_args;

   /* test arguments type */
   if(lua_istable(L, 1))
   {
      num_args = get_arrayd(L, 1, &array);

      if(num_args < 3)
         luaL_error(L, "incorrect argument to function 'gl.Normal'");

      /* call openGL function */
      #ifdef IPHONE
      glNormal3f(array[0], array[1], array[2]);
      #else
      glNormal3dv(array);
      #endif

      free(array);

      return 0;
   }

   /* test arguments */
   if(!( lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3) ))
      luaL_error(L, "incorrect argument to function 'gl.Normal'");

   /* call openGL functions */
   #ifdef IPHONE
   #define glNormal3d glNormal3f
   #endif
   glNormal3d((GLdouble)lua_tonumber(L, 1), (GLdouble)lua_tonumber(L, 2),
              (GLdouble)lua_tonumber(L, 3));

   return 0;
}

/*NormalPointer (normalArray) -> none*/
static int gl_normal_pointer(lua_State *L)
{
   GLint size;

   static GLdouble *array = 0;

   if(array)
      free(array);

   /* test arguments type */
   if(!lua_istable(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.NormalPointer'");
   /* get argument */
   if(get_array2d(L, 1, &array, &size) == -1)
      size = get_arrayd(L, 1, &array) / 3;

   /* call opengl function */
   glNormalPointer(GL_DOUBLE, 0, array);

   return 0;
}

/*Ortho (left, right, bottom, top, zNear, zFar) -> none*/
static int gl_ortho(lua_State *L)
{
   /* test arguments type */
   if(!( lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3) &&
         lua_isnumber(L, 4) && lua_isnumber(L, 5) && lua_isnumber(L, 6)))
      luaL_error(L, "incorrect string argument to function 'gl.Ortho'");

   /* call opengl function */
   #ifdef IPHONE
   #define glOrtho glOrthof
   #endif
   glOrtho((GLdouble)lua_tonumber(L, 1), (GLdouble)lua_tonumber(L, 2),
           (GLdouble)lua_tonumber(L, 3), (GLdouble)lua_tonumber(L, 4),
           (GLdouble)lua_tonumber(L, 5), (GLdouble)lua_tonumber(L, 6));

   return 0;
}

/*PointSize (size) -> none*/
static int gl_point_size(lua_State *L)
{
   /* test arguments type */
   if(!lua_isnumber(L, 1))
      luaL_error(L, "incorrect string argument to function 'gl.PointSize'");

   /* call opengl function */
   glPointSize((GLfloat)lua_tonumber(L, 1));

   return 0;
}

/*PolygonOffset (factor, units) -> none*/
static int gl_polygon_offset(lua_State *L)
{
   /* test arguments type */
   if(!( lua_isnumber(L, 1) && lua_isnumber(L, 2) ))
      luaL_error(L, "incorrect string argument to function 'gl.PolygonOffset'");

   /* call opengl function */
   glPolygonOffset((GLfloat)lua_tonumber(L, 1), (GLfloat)lua_tonumber(L, 2));

   return 0;
}

/*PopMatrix () -> none*/
static int gl_pop_matrix(lua_State *L)
{
   glPopMatrix();
   return 0;
}

/*PushMatrix () -> none*/
static int gl_push_matrix(lua_State *L)
{
   glPushMatrix();
   return 0;
}

/*ReadPixels (x, y, width, height, format, pixelsArray) -> none*/
static int gl_read_pixels(lua_State *L)
{
   GLenum e;
   GLfloat *pixels;

   /* test arguments type */
   if(!(lua_isnumber(L, 1) && lua_isnumber(L, 2) &&
        lua_isnumber(L, 3) && lua_isnumber(L, 4) &&
        lua_isstring(L, 5) && lua_istable (L, 6)) )
      luaL_error(L, "incorrect argument to function 'gl.ReadPixels'");

   /* get parameters */
   e = get_gl_enum(L, 5);
   get_arrayf(L, 6, &pixels);

   /* test argument */
   if(e == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.ReadPixels'");

   /* call opengl function */
   glReadPixels((GLint)lua_tonumber(L, 1), (GLint)lua_tonumber(L, 2),
                (GLsizei)lua_tonumber(L, 3), (GLsizei)lua_tonumber(L, 4),
                e, GL_FLOAT, pixels);

   free(pixels);

   return 0;
}

/*Rotate (angle, x, y, z) -> none*/
static int gl_rotate(lua_State *L)
{
   /* test argument type */
   if(!( lua_isnumber(L, 1) && lua_isnumber(L, 2) &&
         lua_isnumber(L, 3) && lua_isnumber(L, 4) ))
      luaL_error(L, "incorrect argument to function 'gl.Rotate'");

   /* call opengl function */
   #ifdef IPHONE
   #define glRotated glRotatef
   #endif
   glRotated((GLdouble)lua_tonumber(L, 1), (GLdouble)lua_tonumber(L, 2),
             (GLdouble)lua_tonumber(L, 3), (GLdouble)lua_tonumber(L, 4));

   return 0;
}

/*Scale (x, y, z) -> none*/
static int gl_scale(lua_State *L)
{
   /* test argument type */
   if(!( lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3) ))
      luaL_error(L, "incorrect argument to function 'gl.Scale'");

   /* call opengl function */
   #ifdef IPHONE
   #define glScaled glScalef
   #endif
   glScaled((GLdouble)lua_tonumber(L, 1), (GLdouble)lua_tonumber(L, 2),
            (GLdouble)lua_tonumber(L, 3));

   return 0;
}

/*Scissor (x, y, width, height) -> none*/
static int gl_scissor(lua_State *L)
{
   /* test argument type */
   if(!( lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3) && lua_isnumber(L, 4) ))
      luaL_error(L, "incorrect argument to function 'gl.Scissor'");

   /* call opengl function */
   glScissor((GLint)lua_tonumber(L, 1), (GLint)lua_tonumber(L, 2),
             (GLsizei)lua_tonumber(L, 3), (GLsizei)lua_tonumber(L, 4));

   return 0;
}


/*ShadeModel (mode) -> none*/
static int gl_shade_model(lua_State *L)
{
   GLenum mode;

   /* test argument type */
   if(!lua_isstring(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.ShadeModel'");

   /* get string parameter */
   mode = get_gl_enum(L, 1);

   /* test argument */
   if(mode == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.ShadeModel'");

   /* call opengl function */
   glShadeModel(mode);

   return 0;
}

/*StencilFunc (func, ref, mask) -> none*/
static int gl_stencil_func(lua_State *L)
{
   GLenum func;

   /* test arguments type */
   if(!( lua_isstring(L, 1) && lua_isnumber(L, 2) ))
      luaL_error(L, "incorrect argument to function 'gl.StencilFunc'");

   /* get string parameter */
   func = get_gl_enum(L, 1);

   /* test argument */
   if(func == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.StencilFunc'");

   if(lua_type(L,3) == LUA_TSTRING)
      /* call opengl function */
      glStencilFunc(func, (GLint)lua_tonumber(L, 2), str2mask(lua_tostring(L, 3)));

   else if(lua_type(L,3) == LUA_TNUMBER)
      /* call opengl function */
      glStencilFunc(func, (GLint)lua_tonumber(L, 2), (GLuint)lua_tonumber(L, 3));

   else
      luaL_error(L, "incorrect argument to function 'gl.StencilFunc'");

   return 0;
}

/*StencilMask (mask) -> none*/
static int gl_stencil_mask(lua_State *L)
{
   if(lua_type(L,1) == LUA_TSTRING)
      /* call opengl function */
      glStencilMask(str2mask(lua_tostring(L, 1)));

   else if(lua_type(L,1) == LUA_TNUMBER)
      /* call opengl function */
      glStencilMask((GLuint)lua_tonumber(L, 1));

   else
      luaL_error(L, "incorrect argument to function 'gl.StencilMask'");

   return 0;
}

/*StencilOp (fail, zfail, zpass) -> none*/
static int gl_stencil_op(lua_State *L)
{
   GLenum e1, e2, e3;

   /* test arguments type */
   if( !(lua_isstring(L, 1) && lua_isstring(L, 2) && lua_isstring(L, 3) ))
      luaL_error(L, "incorrect argument to function 'gl.StencilOp'");

   e1 = get_gl_enum(L, 1);
   e2 = get_gl_enum(L, 2);
   e3 = get_gl_enum(L, 3);

   /* test argument */
   if(e1 == ENUM_ERROR || e2 == ENUM_ERROR || e3 == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.StencilOp'");

   /* call opengl function */
   glStencilOp(e1, e2, e3);

   return 0;
}

/*TexCoordPointer(vArray) -> none*/
static int gl_tex_coord_pointer(lua_State *L)
{
   GLint size;
   static GLdouble *array = 0;

   if(array)
      free(array);

   /* test arguments type */
   if(!lua_istable(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.TexCoordPointer'");

   if(lua_isnumber(L, 2))
   {
      size = (GLint)lua_tonumber(L, 2);
      get_arrayd(L, 1, &array);
   }
   else if(get_array2d(L, 1, &array, &size) == -1)
      luaL_error(L, "incorrect argument to function 'gl.TexCoordPointer'");

   /* call opengl function */
   glTexCoordPointer(size, GL_DOUBLE, 0, array);

   return 0;
}

/*TexEnv (pname, param) -> none
  TexEnv (pname, paramsArray) -> none*/
int static gl_tex_env(lua_State *L)
{
   GLfloat *param;
   GLenum e;

   /* test arguments type */
   if(!lua_isstring(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.TexEnv'");

   /* get string parameters */
   e = get_gl_enum(L, 1);

   /* test argument */
   if(e == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.TexEnv'");

   if(lua_istable(L, 2))
   {
      get_arrayf(L, 2, &param);

      /* call opengl function */
      glTexEnvfv(GL_TEXTURE_ENV, e, (GLfloat *)param);

      free(param);
   }
   else if(lua_isnumber(L, 2))
      /* call opengl function */
      glTexEnvf(GL_TEXTURE_ENV, e, (GLfloat)lua_tonumber(L, 2));

   else if(lua_isstring(L, 2))
      /* call opengl function */
      glTexEnvi(GL_TEXTURE_ENV, e, get_gl_enum(L, 2));

   else
      luaL_error(L, "incorrect argument to function 'gl.TexEnv'");

   return 0;
}

/*Translate (x, y, z) -> none*/
static int gl_translate(lua_State *L)
{
   /* test arguments type */
   if(!( lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3) ))
      luaL_error(L, "incorrect argument to function 'gl.Translate'");

   /* call opengl function */
   #ifdef IPHONE
   #define glTranslated glTranslatef
   #endif
   glTranslated((GLdouble)lua_tonumber(L, 1), (GLdouble)lua_tonumber(L, 2),
                (GLdouble)lua_tonumber(L, 3));

   return 0;
}

/*VertexPointer (vertexArray) -> none*/
static int gl_vertex_pointer(lua_State *L)
{
   GLint size;
   static GLdouble *array = 0;

   if(array)
      free(array);

   /* test arguments type */
   if(!lua_istable(L, 1))
      luaL_error(L, "incorrect argument to function 'gl.VertexPointer'");

   if(lua_isnumber(L, 2))
   {
      size = (GLint)lua_tonumber(L, 2);
      get_arrayd(L, 1, &array);
   }
   else if(get_array2d(L, 1, &array, &size) == -1)
   {
      luaL_error(L, "incorrect argument to function 'gl.VertexPointer'");
      return 0;
   }

   /* call opengl function */
   glVertexPointer(size, GL_DOUBLE, 0, array);

   return 0;
}

/*Viewport (x, y, width, height) -> none*/
static int gl_viewport(lua_State *L)
{
   /* test arguments type */
   if(!( lua_isnumber(L, 1) && lua_isnumber(L, 2) &&
         lua_isnumber(L, 3) && lua_isnumber(L, 4) ))
      luaL_error(L, "incorrect argument to function 'gl.Viewport'");

   /* call openGL function */
   glViewport((GLint)lua_tonumber(L, 1), (GLint)lua_tonumber(L, 2),
              (GLsizei)lua_tonumber(L, 3), (GLsizei)lua_tonumber(L, 4));

   return 0;
}



static const luaL_reg gllib[] = {
  {"AlphaFunc", gl_alpha_func},
  {"BindTexture", gl_bind_texture},
  {"BlendFunc", gl_blend_func},
  {"Clear", gl_clear},
  {"ClearColor", gl_clear_color},
  {"ClearDepth", gl_clear_depth},
  {"ClearStencil", gl_clear_stencil},
  {"ColorPointer", gl_color_pointer},
  {"CullFace",gl_cull_face},
  {"DepthFunc",gl_depth_func},
  {"DepthMask",gl_depth_mask},
  {"DepthRange",gl_depth_range},
  {"Disable",gl_disable},
  {"DisableClientState",gl_disable_client_state},
  {"DrawArrays",gl_draw_arrays},
  {"DrawElements", gl_draw_elements},
  {"Enable", gl_enable},
  {"EnableClientState", gl_enable_client_state},
  {"Finish", gl_finish},
  {"Flush", gl_flush},
  {"Fog", gl_fog},
  {"FrontFace", gl_front_face},
  {"Frustum", gl_frustum},
  {"GenTextures", gl_gen_textures},
  {"Get", gl_get},
  {"GetClipPlane", gl_get_clip_plane},
  {"GetError", gl_get_error},
  {"GetLight", gl_get_light},
  {"GetMaterial", gl_get_material},
  {"GetPointer", gl_get_pointer},
  {"GetString", gl_get_string},
  {"GetTexEnv", gl_get_tex_env},
  {"GetTexParameter", gl_get_tex_parameter},
  {"Hint", gl_hint},
  {"IsEnabled", gl_is_enabled},
  {"IsTexture", gl_is_texture},
  {"Light", gl_light},
  {"LightModel", gl_light_model},
  {"LineWidth", gl_line_width},
  {"LoadIdentity", gl_load_identity},
  {"LoadMatrix", gl_load_matrix},
  {"LogicOp", gl_logic_op},
  {"Material", gl_material},
  {"MatrixMode", gl_matrix_mode},
  {"MultMatrix", gl_mult_matrix},
  {"Normal", gl_normal},
  {"NormalPointer", gl_normal_pointer},
  {"Ortho", gl_ortho},  
  {"PointSize", gl_point_size},
  {"PolygonOffset", gl_polygon_offset},
  {"PopMatrix", gl_pop_matrix},
  {"ReadPixels", gl_read_pixels},
  {"Rotate", gl_rotate},
  {"Scale", gl_scale},
  {"Scissor", gl_scissor},
  {"ShadeModel", gl_shade_model},
  {"StencilFunc", gl_stencil_func},
  {"StencilMask", gl_stencil_mask},
  {"StencilOp", gl_stencil_op},
  {"TexCoordPointer", gl_tex_coord_pointer},
  {"TexEnv", gl_tex_env},
  {"Translate", gl_translate},
  {"VertexPointer", gl_vertex_pointer},
  {"Viewport", gl_viewport},
  
  {"ClipPlane", gl_clip_plane},
  {"DeleteTextures",gl_delete_textures},
  {"PushMatrix", gl_push_matrix},

  {NULL, NULL}
};


bool tabled = false;
void luagl_table(const gl_str_value *gl_str) {
  if(!tabled) {
    int ofs = 0;
    while(gl_str[ofs].str) {
      if(luagl_string_to_enum.count(gl_str[ofs].str)) {
        dprintf("what what %s\n", gl_str[ofs].str);
        assert(0);
      }
      luagl_string_to_enum[gl_str[ofs].str] = gl_str[ofs].value;
      if(luagl_enum_to_string.count(gl_str[ofs].value) == 0)
        luagl_enum_to_string[gl_str[ofs].value] = gl_str[ofs].str;
      ofs++;
    }
    
    tabled = true;
  }
}

void luaopen_opengl_common (lua_State *L) {
  luaL_openlib(L, "gl", gllib, 0);
  lua_pop(L, 1);
}

map<string, int> luagl_string_to_enum;
map<int, string> luagl_enum_to_string;
