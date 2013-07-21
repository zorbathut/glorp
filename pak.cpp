
#include "pak.h"

#include "unzip.h"
#include "os.h"

#include <sys/types.h>
#include <sys/stat.h>

#include <cstdio>

namespace Glorp {
  unzFile zfile = 0;
  
  void PakInit() {
    zfile = unzOpen(exeName().c_str()); // may end up being null, that's OK
  }
  
  bool PakHas(const std::string &filename) {
    struct _stat buf;
    if (!_stat(filename.c_str(), &buf)) {
      return true;
    }
    
    if (zfile && unzLocateFile(zfile, filename.c_str(), 1) == UNZ_OK) {
      return true;
    }
    
    return false;
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
    
    if (zfile && unzLocateFile(zfile, filename.c_str(), 1) == UNZ_OK) {
      unzOpenCurrentFile(zfile);  // TODO: verify UNZ_OK
      
      char buf[16384];
      while (true) {
        int data = unzReadCurrentFile(zfile, buf, sizeof(buf));
        if (data <= 0) {
          break;
        }
        
        dest->insert(dest->end(), buf, buf + data);
      }
      
      unzCloseCurrentFile(zfile);  // TODO: verify UNZ_OK
      
      return true;
    }
    
    return false;
  }
}
