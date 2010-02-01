
#include "LuaGLES.h"
#include "LuaGL_common.h"


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
         case 3:  glColor4f(array[0], array[1], array[2], 1.0f); break;
         case 4:  glColor4f(array[0], array[1], array[2], array[3]); break;
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
      case 3:  glColor4f((GLdouble)lua_tonumber(L, 1), (GLdouble)lua_tonumber(L, 2),
                         (GLdouble)lua_tonumber(L, 3), 1.0f);
               break;
      case 4:  glColor4f((GLdouble)lua_tonumber(L, 1), (GLdouble)lua_tonumber(L, 2),
                         (GLdouble)lua_tonumber(L, 3), (GLdouble)lua_tonumber(L, 4));
               break;
   }
   return 0;
}
#if 0
/*TexImage(level, internalformat, format, pixels) -> none*/
static int gl_tex_image(lua_State *L)
{
   GLenum e;
   GLfloat *pixels;
   GLsizei width, height;
   int iformat;

   /* test arguments type */
   if(!( lua_isnumber(L, 1) && lua_isnumber(L, 2) &&
         lua_isstring(L, 3) && lua_istable(L, 4) ))
      luaL_error(L, "incorrect argument to function 'gl.TexImage'");

   e = get_gl_enum(L, 3);

   /* test argument */
   if(e == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.TexImage'");

   iformat = (int)lua_tonumber(L, 2);

   if((height = get_array2f(L, 4, &pixels, &width)) != -1)
   {
      glTexImage2D(GL_TEXTURE_2D, (GLint)lua_tonumber(L, 1),
                   iformat, width/iformat, height, 0, e, GL_FLOAT, pixels);
      return 0;
   }
   else
   {
      width = get_arrayf(L, 4, &pixels);
      glTexImage1D(GL_TEXTURE_1D, (GLint)lua_tonumber(L, 1),
                   iformat, width/iformat, 0, e, GL_FLOAT, pixels);
      return 0;
   }
}

/*TexSubImage (level, format, pixels, xoffset) -> none
  TexSubImage (level, format, pixels, xoffset, yoffset) -> none*/
static int gl_tex_sub_image(lua_State *L)
{
   GLenum format;
   GLfloat *pixels;
   GLsizei width, height;
   int size = 1;

   /* test arguments type */
   if(!( lua_isnumber(L, 1) && lua_isstring(L, 2) &&
         lua_istable(L, 3) && lua_isnumber(L, 4) ))
      luaL_error(L, "incorrect argument to function 'gl.TexSubImage'");

   format = get_gl_enum(L, 2);
   switch(format)
   {
      case GL_COLOR_INDEX:
      case GL_RED:
      case GL_GREEN:
      case GL_BLUE:
      case GL_ALPHA:
      case GL_LUMINANCE:
         size = 1;
         break;

      case GL_LUMINANCE_ALPHA:
         size = 2;
         break;

      case GL_RGB:
      //case GL_BGR_EXT:
         size = 3;
         break;

      case GL_RGBA:
      //case GL_BGRA_EXT:
         size = 4;
         break;
   }

   /* test argument */
   if(format == ENUM_ERROR)
      luaL_error(L, "incorrect string argument to function 'gl.TexSubImage'");

   if((height = get_array2f(L, 3, &pixels, &width)) != -1)
   {
      glTexSubImage2D(GL_TEXTURE_2D, (GLint)lua_tonumber(L, 1), (GLint)lua_tonumber(L, 4),
                      (GLint)lua_tonumber(L, 5), width/size, height, format, GL_FLOAT, pixels);
      return 0;
   }
   else
   {
      width = get_arrayf(L, 3, &pixels);
      glTexSubImage1D(GL_TEXTURE_1D, (GLint)lua_tonumber(L, 1), (GLint)lua_tonumber(L, 4),
                      width/size, format, GL_FLOAT, pixels);
      return 0;
   }
}
#endif

static const gl_str_value gl_str[] = {
  MACRIX(NO_ERROR),

  /* ClearBufferMask */
  MACRIX(DEPTH_BUFFER_BIT),
  MACRIX(STENCIL_BUFFER_BIT),
  MACRIX(COLOR_BUFFER_BIT),

  /* Boolean */
  MACRIX(FALSE),
  MACRIX(TRUE),

  /* BeginMode */
  MACRIX(POINTS),
  MACRIX(LINES),
  MACRIX(LINE_LOOP),
  MACRIX(LINE_STRIP),
  MACRIX(TRIANGLES),
  MACRIX(TRIANGLE_STRIP),
  MACRIX(TRIANGLE_FAN),

  /* AlphaFunction */
  MACRIX(NEVER),
  MACRIX(LESS),
  MACRIX(EQUAL),
  MACRIX(LEQUAL),
  MACRIX(GREATER),
  MACRIX(NOTEQUAL),
  MACRIX(GEQUAL),
  MACRIX(ALWAYS),

  /* BlendingFactorDest */
  MACRIX(ZERO),
  MACRIX(ONE),
  MACRIX(SRC_COLOR),
  MACRIX(ONE_MINUS_SRC_COLOR),
  MACRIX(SRC_ALPHA),
  MACRIX(ONE_MINUS_SRC_ALPHA),
  MACRIX(DST_ALPHA),
  MACRIX(ONE_MINUS_DST_ALPHA),

  /* BlendingFactorSrc */
  /*      GL_ZERO */
  /*      GL_ONE */
  MACRIX(DST_COLOR),
  MACRIX(ONE_MINUS_DST_COLOR),
  MACRIX(SRC_ALPHA_SATURATE),
  /*      GL_SRC_ALPHA */
  /*      GL_ONE_MINUS_SRC_ALPHA */
  /*      GL_DST_ALPHA */
  /*      GL_ONE_MINUS_DST_ALPHA */

  /* ClipPlaneName */
  MACRIX(CLIP_PLANE0),
  MACRIX(CLIP_PLANE1),
  MACRIX(CLIP_PLANE2),
  MACRIX(CLIP_PLANE3),
  MACRIX(CLIP_PLANE4),
  MACRIX(CLIP_PLANE5),

  /* ColorMaterialFace */
  /*      GL_FRONT_AND_BACK */

  /* ColorMaterialParameter */
  /*      GL_AMBIENT_AND_DIFFUSE */

  /* ColorPointerType */
  /*      GL_UNSIGNED_BYTE */
  /*      GL_FLOAT */
  /*      GL_FIXED */

  /* CullFaceMode */
  MACRIX(FRONT),
  MACRIX(BACK),
  MACRIX(FRONT_AND_BACK),

  /* DepthFunction */
  /*      GL_NEVER */
  /*      GL_LESS */
  /*      GL_EQUAL */
  /*      GL_LEQUAL */
  /*      GL_GREATER */
  /*      GL_NOTEQUAL */
  /*      GL_GEQUAL */
  /*      GL_ALWAYS */

  /* EnableCap */
  MACRIX(FOG),
  MACRIX(LIGHTING),
  MACRIX(TEXTURE_2D),
  MACRIX(CULL_FACE),
  MACRIX(ALPHA_TEST),
  MACRIX(BLEND),
  MACRIX(COLOR_LOGIC_OP),
  MACRIX(DITHER),
  MACRIX(STENCIL_TEST),
  MACRIX(DEPTH_TEST),
  /*      GL_LIGHT0 */
  /*      GL_LIGHT1 */
  /*      GL_LIGHT2 */
  /*      GL_LIGHT3 */
  /*      GL_LIGHT4 */
  /*      GL_LIGHT5 */
  /*      GL_LIGHT6 */
  /*      GL_LIGHT7 */
  MACRIX(POINT_SMOOTH),
  MACRIX(LINE_SMOOTH),
  MACRIX(COLOR_MATERIAL),
  MACRIX(NORMALIZE),
  MACRIX(RESCALE_NORMAL),
  MACRIX(POLYGON_OFFSET_FILL),
  MACRIX(VERTEX_ARRAY),
  MACRIX(NORMAL_ARRAY),
  MACRIX(COLOR_ARRAY),
  MACRIX(TEXTURE_COORD_ARRAY),
  MACRIX(MULTISAMPLE),
  MACRIX(SAMPLE_ALPHA_TO_COVERAGE),
  MACRIX(SAMPLE_ALPHA_TO_ONE),
  MACRIX(SAMPLE_COVERAGE),

  /* ErrorCode */
  MACRIX(INVALID_ENUM),
  MACRIX(INVALID_VALUE),
  MACRIX(INVALID_OPERATION),
  MACRIX(STACK_OVERFLOW),
  MACRIX(STACK_UNDERFLOW),
  MACRIX(OUT_OF_MEMORY),

  /* FogMode */
  /*      GL_LINEAR */
  MACRIX(EXP),
  MACRIX(EXP2),

  /* FogParameter */
  MACRIX(FOG_DENSITY),
  MACRIX(FOG_START),
  MACRIX(FOG_END),
  MACRIX(FOG_MODE),
  MACRIX(FOG_COLOR),

  /* FrontFaceDirection */
  MACRIX(CW),
  MACRIX(CCW),

  /* GetPName */
  MACRIX(CURRENT_COLOR),
  MACRIX(CURRENT_NORMAL),
  MACRIX(CURRENT_TEXTURE_COORDS),
  MACRIX(POINT_SIZE),
  MACRIX(POINT_SIZE_MIN),
  MACRIX(POINT_SIZE_MAX),
  MACRIX(POINT_FADE_THRESHOLD_SIZE),
  MACRIX(POINT_DISTANCE_ATTENUATION),
  MACRIX(SMOOTH_POINT_SIZE_RANGE),
  MACRIX(LINE_WIDTH),
  MACRIX(SMOOTH_LINE_WIDTH_RANGE),
  MACRIX(ALIASED_POINT_SIZE_RANGE),
  MACRIX(ALIASED_LINE_WIDTH_RANGE),
  MACRIX(CULL_FACE_MODE),
  MACRIX(FRONT_FACE),
  MACRIX(SHADE_MODEL),
  MACRIX(DEPTH_RANGE),
  MACRIX(DEPTH_WRITEMASK),
  MACRIX(DEPTH_CLEAR_VALUE),
  MACRIX(DEPTH_FUNC),
  MACRIX(STENCIL_CLEAR_VALUE),
  MACRIX(STENCIL_FUNC),
  MACRIX(STENCIL_VALUE_MASK),
  MACRIX(STENCIL_FAIL),
  MACRIX(STENCIL_PASS_DEPTH_FAIL),
  MACRIX(STENCIL_PASS_DEPTH_PASS),
  MACRIX(STENCIL_REF),
  MACRIX(STENCIL_WRITEMASK),
  MACRIX(MATRIX_MODE),
  MACRIX(VIEWPORT),
  MACRIX(MODELVIEW_STACK_DEPTH),
  MACRIX(PROJECTION_STACK_DEPTH),
  MACRIX(TEXTURE_STACK_DEPTH),
  MACRIX(MODELVIEW_MATRIX),
  MACRIX(PROJECTION_MATRIX),
  MACRIX(TEXTURE_MATRIX),
  MACRIX(ALPHA_TEST_FUNC),
  MACRIX(ALPHA_TEST_REF),
  MACRIX(BLEND_DST),
  MACRIX(BLEND_SRC),
  MACRIX(LOGIC_OP_MODE),
  MACRIX(SCISSOR_BOX),
  MACRIX(SCISSOR_TEST),
  MACRIX(COLOR_CLEAR_VALUE),
  MACRIX(COLOR_WRITEMASK),
  MACRIX(UNPACK_ALIGNMENT),
  MACRIX(PACK_ALIGNMENT),
  MACRIX(MAX_LIGHTS),
  MACRIX(MAX_CLIP_PLANES),
  MACRIX(MAX_TEXTURE_SIZE),
  MACRIX(MAX_MODELVIEW_STACK_DEPTH),
  MACRIX(MAX_PROJECTION_STACK_DEPTH),
  MACRIX(MAX_TEXTURE_STACK_DEPTH),
  MACRIX(MAX_VIEWPORT_DIMS),
  MACRIX(MAX_TEXTURE_UNITS),
  MACRIX(SUBPIXEL_BITS),
  MACRIX(RED_BITS),
  MACRIX(GREEN_BITS),
  MACRIX(BLUE_BITS),
  MACRIX(ALPHA_BITS),
  MACRIX(DEPTH_BITS),
  MACRIX(STENCIL_BITS),
  MACRIX(POLYGON_OFFSET_UNITS),
  MACRIX(POLYGON_OFFSET_FACTOR),
  MACRIX(TEXTURE_BINDING_2D),
  MACRIX(VERTEX_ARRAY_SIZE),
  MACRIX(VERTEX_ARRAY_TYPE),
  MACRIX(VERTEX_ARRAY_STRIDE),
  MACRIX(NORMAL_ARRAY_TYPE),
  MACRIX(NORMAL_ARRAY_STRIDE),
  MACRIX(COLOR_ARRAY_SIZE),
  MACRIX(COLOR_ARRAY_TYPE),
  MACRIX(COLOR_ARRAY_STRIDE),
  MACRIX(TEXTURE_COORD_ARRAY_SIZE),
  MACRIX(TEXTURE_COORD_ARRAY_TYPE),
  MACRIX(TEXTURE_COORD_ARRAY_STRIDE),
  MACRIX(VERTEX_ARRAY_POINTER),
  MACRIX(NORMAL_ARRAY_POINTER),
  MACRIX(COLOR_ARRAY_POINTER),
  MACRIX(TEXTURE_COORD_ARRAY_POINTER),
  MACRIX(SAMPLE_BUFFERS),
  MACRIX(SAMPLES),
  MACRIX(SAMPLE_COVERAGE_VALUE),
  MACRIX(SAMPLE_COVERAGE_INVERT),

  /* GetTextureParameter */
  /*      GL_TEXTURE_MAG_FILTER */
  /*      GL_TEXTURE_MIN_FILTER */
  /*      GL_TEXTURE_WRAP_S */
  /*      GL_TEXTURE_WRAP_T */

  MACRIX(IMPLEMENTATION_COLOR_READ_TYPE_OES),
  MACRIX(IMPLEMENTATION_COLOR_READ_FORMAT_OES),
  MACRIX(NUM_COMPRESSED_TEXTURE_FORMATS),
  MACRIX(COMPRESSED_TEXTURE_FORMATS),

  /* HintMode */
  MACRIX(DONT_CARE),
  MACRIX(FASTEST),
  MACRIX(NICEST),

  /* HintTarget */
  MACRIX(PERSPECTIVE_CORRECTION_HINT),
  MACRIX(POINT_SMOOTH_HINT),
  MACRIX(LINE_SMOOTH_HINT),
  MACRIX(FOG_HINT),
  MACRIX(GENERATE_MIPMAP_HINT),

  /* LightModelParameter */
  MACRIX(LIGHT_MODEL_AMBIENT),
  MACRIX(LIGHT_MODEL_TWO_SIDE),

  /* LightParameter */
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

  /* DataType */
  MACRIX(BYTE),
  MACRIX(UNSIGNED_BYTE),
  MACRIX(SHORT),
  MACRIX(UNSIGNED_SHORT),
  MACRIX(FLOAT),
  MACRIX(FIXED),

  /* LogicOp */
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

  /* MaterialFace */
  /*      GL_FRONT_AND_BACK */

  /* MaterialParameter */
  MACRIX(EMISSION),
  MACRIX(SHININESS),
  MACRIX(AMBIENT_AND_DIFFUSE),
  /*      GL_AMBIENT */
  /*      GL_DIFFUSE */
  /*      GL_SPECULAR */

  /* MatrixMode */
  MACRIX(MODELVIEW),
  MACRIX(PROJECTION),
  MACRIX(TEXTURE),

  /* NormalPointerType */
  /*      GL_BYTE */
  /*      GL_SHORT */
  /*      GL_FLOAT */
  /*      GL_FIXED */

  /* PixelFormat */
  MACRIX(ALPHA),
  MACRIX(RGB),
  MACRIX(RGBA),
  MACRIX(LUMINANCE),
  MACRIX(LUMINANCE_ALPHA),

  /* PixelStoreParameter */

  /* PixelType */
  /*      GL_UNSIGNED_BYTE */
  MACRIX(UNSIGNED_SHORT_4_4_4_4),
  MACRIX(UNSIGNED_SHORT_5_5_5_1),
  MACRIX(UNSIGNED_SHORT_5_6_5),

  /* ShadingModel */
  MACRIX(FLAT),
  MACRIX(SMOOTH),

  /* StencilFunction */
  /*      GL_NEVER */
  /*      GL_LESS */
  /*      GL_EQUAL */
  /*      GL_LEQUAL */
  /*      GL_GREATER */
  /*      GL_NOTEQUAL */
  /*      GL_GEQUAL */
  /*      GL_ALWAYS */

  /* StencilOp */
  /*      GL_ZERO */
  MACRIX(KEEP),
  MACRIX(REPLACE),
  MACRIX(INCR),
  MACRIX(DECR),
  /*      GL_INVERT */

  /* StringName */
  MACRIX(VENDOR),
  MACRIX(RENDERER),
  MACRIX(VERSION),
  MACRIX(EXTENSIONS),

  /* TexCoordPointerType */
  /*      GL_SHORT */
  /*      GL_FLOAT */
  /*      GL_FIXED */
  /*      GL_BYTE */

  /* TextureEnvMode */
  MACRIX(MODULATE),
  MACRIX(DECAL),
  /*      GL_BLEND */
  MACRIX(ADD),
  /*      GL_REPLACE */

  /* TextureEnvParameter */
  MACRIX(TEXTURE_ENV_MODE),
  MACRIX(TEXTURE_ENV_COLOR),

  /* TextureEnvTarget */
  MACRIX(TEXTURE_ENV),

  /* TextureMagFilter */
  MACRIX(NEAREST),
  MACRIX(LINEAR),

  /* TextureMinFilter */
  /*      GL_NEAREST */
  /*      GL_LINEAR */
  MACRIX(NEAREST_MIPMAP_NEAREST),
  MACRIX(LINEAR_MIPMAP_NEAREST),
  MACRIX(NEAREST_MIPMAP_LINEAR),
  MACRIX(LINEAR_MIPMAP_LINEAR),

  /* TextureParameterName */
  MACRIX(TEXTURE_MAG_FILTER),
  MACRIX(TEXTURE_MIN_FILTER),
  MACRIX(TEXTURE_WRAP_S),
  MACRIX(TEXTURE_WRAP_T),
  MACRIX(GENERATE_MIPMAP),

  /* TextureTarget */
  /*      GL_TEXTURE_2D */

  /* TextureUnit */
  MACRIX(TEXTURE0),
  MACRIX(TEXTURE1),
  MACRIX(TEXTURE2),
  MACRIX(TEXTURE3),
  MACRIX(TEXTURE4),
  MACRIX(TEXTURE5),
  MACRIX(TEXTURE6),
  MACRIX(TEXTURE7),
  MACRIX(TEXTURE8),
  MACRIX(TEXTURE9),
  MACRIX(TEXTURE10),
  MACRIX(TEXTURE11),
  MACRIX(TEXTURE12),
  MACRIX(TEXTURE13),
  MACRIX(TEXTURE14),
  MACRIX(TEXTURE15),
  MACRIX(TEXTURE16),
  MACRIX(TEXTURE17),
  MACRIX(TEXTURE18),
  MACRIX(TEXTURE19),
  MACRIX(TEXTURE20),
  MACRIX(TEXTURE21),
  MACRIX(TEXTURE22),
  MACRIX(TEXTURE23),
  MACRIX(TEXTURE24),
  MACRIX(TEXTURE25),
  MACRIX(TEXTURE26),
  MACRIX(TEXTURE27),
  MACRIX(TEXTURE28),
  MACRIX(TEXTURE29),
  MACRIX(TEXTURE30),
  MACRIX(TEXTURE31),
  MACRIX(ACTIVE_TEXTURE),
  MACRIX(CLIENT_ACTIVE_TEXTURE),

  /* TextureWrapMode */
  MACRIX(REPEAT),
  MACRIX(CLAMP_TO_EDGE),

  /* PixelInternalFormat */
  MACRIX(PALETTE4_RGB8_OES),
  MACRIX(PALETTE4_RGBA8_OES),
  MACRIX(PALETTE4_R5_G6_B5_OES),
  MACRIX(PALETTE4_RGBA4_OES),
  MACRIX(PALETTE4_RGB5_A1_OES),
  MACRIX(PALETTE8_RGB8_OES),
  MACRIX(PALETTE8_RGBA8_OES),
  MACRIX(PALETTE8_R5_G6_B5_OES),
  MACRIX(PALETTE8_RGBA4_OES),
  MACRIX(PALETTE8_RGB5_A1_OES),

  /* VertexPointerType */
  /*      GL_SHORT */
  /*      GL_FLOAT */
  /*      GL_FIXED */
  /*      GL_BYTE */

  /* LightName */
  MACRIX(LIGHT0),
  MACRIX(LIGHT1),
  MACRIX(LIGHT2),
  MACRIX(LIGHT3),
  MACRIX(LIGHT4),
  MACRIX(LIGHT5),
  MACRIX(LIGHT6),
  MACRIX(LIGHT7),

  /* Buffer Objects */
  MACRIX(ARRAY_BUFFER),
  MACRIX(ELEMENT_ARRAY_BUFFER),

  MACRIX(ARRAY_BUFFER_BINDING),
  MACRIX(ELEMENT_ARRAY_BUFFER_BINDING),
  MACRIX(VERTEX_ARRAY_BUFFER_BINDING),
  MACRIX(NORMAL_ARRAY_BUFFER_BINDING),
  MACRIX(COLOR_ARRAY_BUFFER_BINDING),
  MACRIX(TEXTURE_COORD_ARRAY_BUFFER_BINDING),

  MACRIX(STATIC_DRAW),
  MACRIX(DYNAMIC_DRAW),

  MACRIX(BUFFER_SIZE),
  MACRIX(BUFFER_USAGE),

  /* Texture combine + dot3 */
  MACRIX(SUBTRACT),
  MACRIX(COMBINE),
  MACRIX(COMBINE_RGB),
  MACRIX(COMBINE_ALPHA),
  MACRIX(RGB_SCALE),
  MACRIX(ADD_SIGNED),
  MACRIX(INTERPOLATE),
  MACRIX(CONSTANT),
  MACRIX(PRIMARY_COLOR),
  MACRIX(PREVIOUS),
  MACRIX(OPERAND0_RGB),
  MACRIX(OPERAND1_RGB),
  MACRIX(OPERAND2_RGB),
  MACRIX(OPERAND0_ALPHA),
  MACRIX(OPERAND1_ALPHA),
  MACRIX(OPERAND2_ALPHA),

  MACRIX(ALPHA_SCALE),

  MACRIX(SRC0_RGB),
  MACRIX(SRC1_RGB),
  MACRIX(SRC2_RGB),
  MACRIX(SRC0_ALPHA),
  MACRIX(SRC1_ALPHA),
  MACRIX(SRC2_ALPHA),

  MACRIX(DOT3_RGB),
  MACRIX(DOT3_RGBA),

  /*****************************************************************************************/
  /*                                 OES extension functions                               */
  /*****************************************************************************************/

  /* OES_draw_texture */
  MACRIX(TEXTURE_CROP_RECT_OES),

  /* OES_matrix_get */
  MACRIX(MODELVIEW_MATRIX_FLOAT_AS_INT_BITS_OES),
  MACRIX(PROJECTION_MATRIX_FLOAT_AS_INT_BITS_OES),
  MACRIX(TEXTURE_MATRIX_FLOAT_AS_INT_BITS_OES),

  /* OES_matrix_palette */
  MACRIX(MAX_VERTEX_UNITS_OES),
  MACRIX(MAX_PALETTE_MATRICES_OES),
  MACRIX(MATRIX_PALETTE_OES),
  MACRIX(MATRIX_INDEX_ARRAY_OES),
  MACRIX(WEIGHT_ARRAY_OES),
  MACRIX(CURRENT_PALETTE_MATRIX_OES),

  MACRIX(MATRIX_INDEX_ARRAY_SIZE_OES),
  MACRIX(MATRIX_INDEX_ARRAY_TYPE_OES),
  MACRIX(MATRIX_INDEX_ARRAY_STRIDE_OES),
  MACRIX(MATRIX_INDEX_ARRAY_POINTER_OES),
  MACRIX(MATRIX_INDEX_ARRAY_BUFFER_BINDING_OES),

  MACRIX(WEIGHT_ARRAY_SIZE_OES),
  MACRIX(WEIGHT_ARRAY_TYPE_OES),
  MACRIX(WEIGHT_ARRAY_STRIDE_OES),
  MACRIX(WEIGHT_ARRAY_POINTER_OES),
  MACRIX(WEIGHT_ARRAY_BUFFER_BINDING_OES),

  /* OES_point_size_array */
  MACRIX(POINT_SIZE_ARRAY_OES),
  MACRIX(POINT_SIZE_ARRAY_TYPE_OES),
  MACRIX(POINT_SIZE_ARRAY_STRIDE_OES),
  MACRIX(POINT_SIZE_ARRAY_POINTER_OES),
  MACRIX(POINT_SIZE_ARRAY_BUFFER_BINDING_OES),

  /* OES_point_sprite */
  MACRIX(POINT_SPRITE_OES),
  MACRIX(COORD_REPLACE_OES),  
   { 0, 0}
};


static const luaL_reg gllib[] = {
  {"Color", gl_color},
#if 0
  {"TexImage", gl_tex_image},
  {"TexSubImage", gl_tex_sub_image},
#endif
  {NULL, NULL}
};


int luaopen_opengles(lua_State *L) {
  luagl_table(gl_str);
  
  luaopen_opengl_common(L);
  
  luaL_openlib(L, "gl", gllib, 0);
  return 1;
}

