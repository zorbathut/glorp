
#include "args.h"
#include "core.h"
#include "debug.h"
#include "os.h"
#include "version.h"

#include "GLee.h"

DEFINE_bool(help, false, "Get help");
DEFINE_bool(development, false, "Development tools");
DEFINE_bool(permit_ogl1, false, "Permit OpenGL 1.0 renderer. Note this may crash in many fascinating manners or fail to render important parts of the game, this is strictly for local testing, here there be dragons");
DEFINE_bool(sound, true, "Enable sound");

namespace Glorp {

  void glErrorCheck() {
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
      CHECK(false, "OPENGL ERROR - %d", err);
    }
  }

  /*static*/ bool Core::Prestartup() {
    if(FLAGS_help) {
      map<string, string> flags = getFlagDescriptions();
      #undef printf
      printf("note: CLI help does not really exist in any meaningful enduser fashion, sorry, but that's OK 'cause there aren't really any usable enduser parameters\n");
      for(map<string, string>::iterator itr = flags.begin(); itr != flags.end(); itr++) {
        dprintf("%s: %s\n", itr->first.c_str(), itr->second.c_str());
        printf("%s: %s\n", itr->first.c_str(), itr->second.c_str());
      }
      #define printf FAILURE
      beginShutdown();
      return true;
    }
    return false;
  }

  Core::Core() : m_alcDevice(0), m_alcContext(0) {
    if(FLAGS_sound) {
      m_alcDevice = alcOpenDevice(NULL);
      if(m_alcDevice) {
        m_alcContext = alcCreateContext(m_alcDevice, NULL);
        CHECK(m_alcContext);
        if (m_alcContext) {
          alcMakeContextCurrent(m_alcContext);
          CHECK(alGetError() == AL_NO_ERROR);
          
          m_audioEnabled = true;
        }
      }
    }
  }

  Core::~Core() {
    alcMakeContextCurrent(NULL);
    if (m_alcContext) {
      alcDestroyContext(m_alcContext);
      m_alcContext = NULL;
    }
    if (m_alcDevice) {
      alcCloseDevice(m_alcDevice);
      m_alcDevice = NULL;
    }
  }

  void Core::Event(const KeyEvent &event) {
  }

  Core::UpdateResult Core::Update() {
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
