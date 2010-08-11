
// data we need to provide:
// sx, sy
// dx, dy
// ox, oy
// wx

// line height

#include "init.h"
#include "debug.h"

#include <ft2build.h>
#include FT_FREETYPE_H 
#include <freetype/ftbitmap.h>

#include <vector>
#include <algorithm>

using namespace std;

struct Image {
  vector<vector<unsigned char> > dat;
  
  void resize(int x, int y) {
    dat.resize(y);
    for(int i = 0; i < y; i++)
      dat[i].resize(x);
  }
  void copyfrom(const Image &img, int ox, int oy) {
    CHECK(img.dat[0].size() + ox <= dat[0].size());
    CHECK(img.dat.size() + oy <= dat.size());
    
    for(int y = 0; y < img.dat.size(); y++)
      copy(img.dat[y].begin(), img.dat[y].end(), dat[y + oy].begin() + ox);
  }
  void set(int sx, int sy, int ex, int ey, unsigned char fil) {
    for(int y = sy; y < ey; y++)
      fill(dat[y].begin() + sx, dat[y].begin() + ex, fil);
  }
};
bool operator<(const Image &lhs, const Image &rhs) {
  return lhs.dat.size() * lhs.dat[0].size() > rhs.dat.size() * rhs.dat[0].size();
}

struct Bucket {
  vector<pair<Image, int> > items;
  
  void AddItem(int id, const Image &img) {
    items.push_back(make_pair(img, id));
  }
  void Resolve() {
    const int wid = 512;
    
    Image masq;
    Image dest;
    masq.resize(512, 1);
    dest.resize(512, 1);
    
    sort(items.begin(), items.end());
    
    for(int i = 0; i < items.size(); i++) {
      int idx = items[i].first.dat[0].size();
      int idy = items[i].first.dat.size();
      
      CHECK(idx <= wid);
      
      bool found = false;
      
      for(int ty = 0; ty < 2048 && !found; ty++) {  // hurfing, occasional durfing
        if(ty + idy > dest.dat.size()) {
          masq.resize(512, ty + idy);
          dest.resize(512, ty + idy);
        }
        for(int tx = 0; tx <= wid - items[i].first.dat[0].size() && !found; tx++) {
          bool valid = !(masq.dat[ty][tx] || masq.dat[ty + idy - 1][tx] || masq.dat[ty][tx + idx - 1] || masq.dat[ty + idy - 1][tx + idx - 1]);
          
          if(valid) {
            for(int ity = 0; ity < idy && valid; ity++)
              for(int itx = 0; itx < idx && valid; itx++)
                if(masq.dat[ty + ity][tx + itx])
                  valid = false;
          }

          if(valid) {
            dest.copyfrom(items[i].first, tx, ty);
            masq.set(tx, ty, tx + idx, ty + idy, 255);
            
            found = true;
            
            dprintf("Placed %d at %dx%d-%dx%d\n", items[i].second, tx, ty, tx + idx, ty + idy);
          }
        }
      }
    }
    
    // save images here
  }
};

int main(int argc, char **argv) {
  initProgram(&argc, const_cast<const char ***>(&argv));
  
  Bucket bucket;
  
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
    if(font->glyph->bitmap.buffer) {
      FT_Bitmap tempbitmap;
      FT_Bitmap_New(&tempbitmap);
      FT_Bitmap_Convert(freetype, &font->glyph->bitmap, &tempbitmap, 1);
      
      Image img;
      img.resize(tempbitmap.width + 2, tempbitmap.rows + 2);
      
      for(int y = 0; y < tempbitmap.rows; y++) {
        string ro;
        for(int x = 0; x < tempbitmap.width; x++) {
          ro += (tempbitmap.buffer[x + y * tempbitmap.width] ? "#" : " ");
          img.dat[y + 1][x + 1] = tempbitmap.buffer[x + y * tempbitmap.width];
        }
        dprintf("%s\n", ro.c_str());
      }
      
      bucket.AddItem(kar, img);
       
      FT_Bitmap_Done(freetype, &tempbitmap);
    }
  }
  
  dprintf("resolve\n");
  bucket.Resolve();
}
