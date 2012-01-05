
#include <AL/al.h>
#include <AL/alc.h>

#include "args.h"

DEFINE_bool(help, false, "Get help");
DEFINE_bool(development, false, "Development tools");
DEFINE_bool(permit_ogl1, false, "Permit OpenGL 1.0 renderer. Note this may crash in many fascinating manners or fail to render important parts of the game, this is strictly for local testing, here there be dragons");
DEFINE_bool(sound, true, "Enable sound");

void main(int argc, const char **argv) {
  setInitFlagFile("glorp/settings");
  initProgram(&argc, const_cast<const char ***>(&argv));
  
  if(FLAGS_help) {
    map<string, string> flags = getFlagDescriptions();
    #undef printf
    printf("note: CLI help does not really exist in any meaningful enduser fashion, sorry, but that's OK 'cause there aren't really any usable enduser parameters\n");
    for(map<string, string>::iterator itr = flags.begin(); itr != flags.end(); itr++) {
      dprintf("%s: %s\n", itr->first.c_str(), itr->second.c_str());
      printf("%s: %s\n", itr->first.c_str(), itr->second.c_str());
    }
    return;
  }
  
  ALCdevice* pDevice = NULL;
  ALCcontext* pContext = NULL;
  if(FLAGS_sound) {
    pDevice = alcOpenDevice(NULL);
    if(pDevice) {
      CHECK(pDevice);
      ALCcontext* pContext = alcCreateContext(pDevice, NULL);
      CHECK(pContext);
      alcMakeContextCurrent(pContext);
      CHECK(alGetError() == AL_NO_ERROR);
      
      audio_enabled = true;
    }
  }

  OS::DoMainLoop();
  
  alcMakeContextCurrent(NULL);
  if (pContext) {
    alcDestroyContext(pContext);
    pContext = NULL;
  }
  if (pDevice) {
    alcCloseDevice(pDevice);
    pDevice = NULL;
  }
}

