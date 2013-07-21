#ifndef GLORP_PAK
#define GLORP_PAK

#include <string>
#include <vector>

namespace Glorp {
  void PakInit();
  bool PakHas(const std::string &filename);
  bool PakRead(const std::string &filename, std::vector<unsigned char> *dest);
};

#endif
