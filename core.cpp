
#include "core.h"
#include "debug.h"
#include "version.h"

#include "GLee.h"

void glErrorCheck() {
  GLenum err = glGetError();
  if (err != GL_NO_ERROR)
  {
    CHECK(false, "OPENGL ERROR - %d", err);
  }
}

namespace Glorp {
  Core::Core() { }
  Core::~Core() { }

  void Core::Event(const KeyEvent &event) {
    dprintf("%d %d/%d %d/%d (%c)", event.timestamp, event.mouse_x, event.mouse_y, event.key.index, event.pressed, event.key.index); 
  }

  Core::UpdateResult Core::Update() {
    Sleep(1);
    return UR_RENDER;
  }
  
  void Core::Render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glErrorCheck();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glTranslatef(-1, 1, -1);
    glScalef(2.0f / Version::gameXres, -2.0f / Version::gameYres, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glBegin(GL_TRIANGLES);
    glColor3f(1, 1, 1);
    glVertex3f(0, 0, 0);
    glVertex3f(100, 0, 0);
    glVertex3f(0, 100, 0);
    glEnd();
  }
}

/*
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

*/