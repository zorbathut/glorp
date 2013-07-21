
#include "pak.h"

#include <sys/types.h>
#include <sys/stat.h>

#include <cstdio>

namespace Glorp {
  void PakInit() { }
  
  bool PakHas(const std::string &filename) {
    struct _stat buf;
    return !_stat(filename.c_str(), &buf);
  }
  
  bool PakRead(const std::string &filename, std::vector<unsigned char> *dest) {
    FILE *fil = std::fopen(filename.c_str(), "rb");
    if (fil) {
      char buf[16384];
      while (true) {
        int count = std::fread(buf, 1, sizeof(buf), fil);
        if (count <= 0) {
          break;
        }
        dest->insert(dest->end(), buf, buf + count);
      }
      
      std::fclose(fil);
      
      return true;
    }
    
    return false;
  }
}
