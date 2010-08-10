
#include "init.h"
#include "debug.h"

#include <ft2build.h>
#include FT_FREETYPE_H 
#include <freetype/ftbitmap.h>

int main(int argc, char **argv) {
  initProgram(&argc, const_cast<const char ***>(&argv));
  
  CHECK(argc == 2);
  
  dprintf("%s\n", argv[1]);
  
  FT_Library freetype;
  CHECK(FT_Init_FreeType(&freetype) == 0);
  
  FT_Face font;
  CHECK(FT_New_Face(freetype, argv[1], 0, &font) == 0);
  
  dprintf("%d glyphs, %08x flags, %d units, %d strikes\n", font->num_glyphs, font->face_flags, font->units_per_EM, font->num_fixed_sizes);
  
  CHECK(FT_Set_Pixel_Sizes(font, 0, 32) == 0);
  
  for(int kar = 32; kar < 128; kar++) {
    CHECK(FT_Load_Char(font, kar, FT_LOAD_RENDER|FT_LOAD_MONOCHROME) == 0);
    
    dprintf("%dx%d %08x\n", font->glyph->bitmap.width, font->glyph->bitmap.rows, font->glyph->bitmap.buffer);
    
    FT_Bitmap tempbitmap;
    FT_Bitmap_New(&tempbitmap);
    FT_Bitmap_Convert(freetype, &font->glyph->bitmap, &tempbitmap, 1);
     
    for(int y = 0; y < tempbitmap.rows; y++) {
      string ro;
      for(int x = 0; x < tempbitmap.width; x++) {
        ro += (tempbitmap.buffer[x + y * tempbitmap.width] ? "#" : " ");
      }
      dprintf("%s\n", ro.c_str());
    }
     
    FT_Bitmap_Done(freetype, &tempbitmap);
  }
}
