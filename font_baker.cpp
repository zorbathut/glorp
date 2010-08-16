
// data we need to provide:
// sx, sy
// dx, dy
// ox, oy
// wx

// line height
// dist-per-pixel

#include "init.h"
#include "debug.h"

#include <ft2build.h>
#include FT_FREETYPE_H 
#include <freetype/ftbitmap.h>

#include <png.h>

#include <vector>
#include <algorithm>

#include <cmath>

using namespace std;

const int pixheight = 48;
const int supersample = 32;
const int distmult = 64;  // this is "one pixel in the final version equals 64 difference". reduce this number to increase the "blur" radius, increase it to make things "sharper"

const int maxsearch = (128 * supersample + distmult - 1) / distmult;

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

struct Data {
  // item ID
  int id;
  
  // dimensions of its spot in the world
  int sx, sy, ex, ey;
  
  // offset from the origin
  float ox, oy;
  
  // distance to move the origin forward
  float wx;
};
bool operator<(const Data &lhs, const Data &rhs) {
  return lhs.id < rhs.id; // should be unique
}

string out_prefix;

struct Bucket {
  vector<pair<Image, Data> > items;
  
  void AddItem(const Image &img, const Data &dat) {
    items.push_back(make_pair(img, dat));
  }
  vector<Data> Resolve() {
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
            
            items[i].second.sx = tx;
            items[i].second.sy = ty;
            
            items[i].second.ex = tx + idx;
            items[i].second.ey = ty + idy;
            
            found = true;
            
            dprintf("Placed %d at %dx%d-%dx%d\n", items[i].second, tx, ty, tx + idx, ty + idy);
          }
        }
      }
    }
    
    FILE *fil = fopen((out_prefix + "font.png").c_str(), "wb");
    
    png_structp  png_ptr;
    png_infop  info_ptr;
  
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    CHECK(png_ptr);
  
    info_ptr = png_create_info_struct(png_ptr);
    CHECK(info_ptr);

    png_init_io(png_ptr, fil);
    
    png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);
    
    png_set_IHDR(png_ptr, info_ptr, dest.dat[0].size(), dest.dat.size(), 8, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    
    png_write_info(png_ptr, info_ptr);
    
    for(int y = 0; y < dest.dat.size(); y++) {
      png_write_row(png_ptr, &dest.dat[y][0]);
    }
    
    png_write_end(png_ptr, NULL);
    
    png_destroy_write_struct(&png_ptr, &info_ptr);
    
    vector<Data> dats;
    for(int i = 0; i < items.size(); i++)
      dats.push_back(items[i].second);
    return dats;
  }
};

struct Closest {
  FT_Bitmap bmp;
  
  Closest(FT_Bitmap bmp) : bmp(bmp) { }
  
  float find_closest(int x, int y, char search) {
    int best = 1 << 30;
    for(int i = 1; i <= maxsearch; i++) {
      if(i * i >= best)
        break;
      for(int f = -i; f < i; f++) {
        int dist = i * i + f * f;
        if(dist >= best) continue;
        
        if(safe_access(x + i, y + f) == search || safe_access(x - f, y + i) == search || safe_access(x - i, y - f) == search || safe_access(x + f, y - i) == search)
          best = dist;
      }
    }
    return sqrt(best);
  }
  char safe_access(int x, int y) {
    if(x < 0 || y < 0 || x >= bmp.width || y >= bmp.rows)
      return 0;
    return bmp.buffer[x + y * bmp.width];
  }
};

int main(int argc, char **argv) {
  initProgram(&argc, const_cast<const char ***>(&argv));
  
  Bucket bucket;
  
  CHECK(argc == 3);
  
  dprintf("%s\n", argv[1]);
  out_prefix = argv[2];
  
  FT_Library freetype;
  CHECK(FT_Init_FreeType(&freetype) == 0);
  
  FT_Face font;
  CHECK(FT_New_Face(freetype, argv[1], 0, &font) == 0);
  
  dprintf("%d glyphs, %08x flags, %d units, %d strikes\n", font->num_glyphs, font->face_flags, font->units_per_EM, font->num_fixed_sizes);
  
  CHECK(FT_Set_Pixel_Sizes(font, 0, pixheight * supersample) == 0);
  
  for(int kar = 32; kar < 128; kar++) {
    CHECK(FT_Load_Char(font, kar, FT_LOAD_RENDER|FT_LOAD_MONOCHROME) == 0);
    
    dprintf("%dx%d %08x\n", font->glyph->bitmap.width, font->glyph->bitmap.rows, font->glyph->bitmap.buffer);
    
    Image img;
    
    const int bord = (128 + distmult - 1) / distmult + 1;
    
    if(font->glyph->bitmap.buffer) {
      FT_Bitmap tempbitmap;
      FT_Bitmap_New(&tempbitmap);
      FT_Bitmap_Convert(freetype, &font->glyph->bitmap, &tempbitmap, 1);
      
      Closest closest(tempbitmap);
      
      img.resize((tempbitmap.width + supersample - 1) / supersample + bord * 2, (tempbitmap.rows + supersample - 1) / supersample + bord * 2);
      
      int lmx = img.dat[0].size();
      int lmy = img.dat.size();
      
      for(int y = 0; y < lmy; y++) {
        int cty = (y - bord) * supersample + supersample / 2;
        for(int x = 0; x < lmx; x++) {
          int ctx = (x - bord) * supersample + supersample / 2;
          float dist;
          if(closest.safe_access(ctx, cty)) {
            dist = closest.find_closest(ctx, cty, 0);
          } else {
            dist = -closest.find_closest(ctx, cty, 1);
          }
          
          dist = dist / supersample * distmult + 127;
          
          dist = floor(dist + 0.5);
          
          if(dist < 0) dist = 0;
          if(dist > 255) dist = 255;
          
          img.dat[y][x] = (unsigned char)dist;
        }
      }
      
      FT_Bitmap_Done(freetype, &tempbitmap);
    } else {
      img.resize(1, 1);
    }
    
    Data dat;
    
    dat.id = kar;
    
    dat.sx = 0;
    dat.sy = 0;
    dat.ex = img.dat[0].size();
    dat.ey = img.dat.size();
    
    dat.ox = (float)font->glyph->metrics.horiBearingX / 64 / supersample - bord;
    dat.oy = -(float)font->glyph->metrics.horiBearingY / 64 / supersample - bord;
    
    dat.wx = (float)font->glyph->metrics.horiAdvance / 64 / supersample;
    
    bucket.AddItem(img, dat);
  }
  
  dprintf("resolve\n");
  vector<Data> results = bucket.Resolve();
  
  sort(results.begin(), results.end());
  
  FILE *fil = fopen((out_prefix + "font.lua").c_str(), "wb");
  fprintf(fil, "height = %f\n", -1.f);
  fprintf(fil, "baseline = %f\n", -1.f);
  fprintf(fil, "distslope = %f\n", -1.f);
  fprintf(fil, "characters = {}\n");
  for(int i = 0; i < results.size(); i++) {
    fprintf(fil, "characters[%d] = {sx = %d, sy = %d, ex = %d, ey = %d, ox = %f, oy = %f, w = %f}\n", results[i].id, results[i].sx, results[i].sy, results[i].ex, results[i].ey, results[i].ox, results[i].oy, results[i].wx);
  }
  fclose(fil);
}
