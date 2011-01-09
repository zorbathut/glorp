#ifndef GLORP_CORE
#define GLORP_CORE

#include <string>
#include <Glop/Sound.h>

using namespace std;

void glorp_init(const string &name, int x, int y, int argc, const char **argv);

SoundSample *SSLoad(const string &fname_base, float vol);

#endif
