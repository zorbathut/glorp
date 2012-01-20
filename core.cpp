
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

  Core::Core() :
        m_alcDevice(0),
        m_alcContext(0),
        m_audioEnabled(false),
        m_L(0),
        m_luaCrashed(false),
        m_event_system_update_begin(LUA_NOREF),
        m_event_system_update_end(LUA_NOREF),
        m_event_system_mouse(LUA_NOREF),
        m_event_system_key(LUA_NOREF) {
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
    l_init();
  }

  Core::~Core() {
    l_shutdown();
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
    if (FLAGS_development && event.key == Keys::F12 && event.pressed) {
      l_shutdown();
      l_init();
    } else {
      // lua event here
    }
  }

  Core::UpdateResult Core::Update() {
    // I don't even know what we do here, honestly. Figure this out later when we have a more complex rendering path?
    return UR_RENDER;
  }
  
  void Core::Render() {
    if (m_L && !m_luaCrashed) {
       l_callEvent(m_L, m_event_system_update_begin);
      glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
      // call the UI layer here, *not* lua
      l_callEvent(m_L, m_event_system_update_end);
    } else {
      glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glScalef(2.0f / Version::gameXres, -2.0f / Version::gameYres, 1.0f);
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();

      glBegin(GL_QUADS);
      int scal = 50;
      for(int x = -5; x < 5; x++) {
        for(int y = -2; y < 2; y++) {
          if((x + y) % 2 == 0) {
            glColor3d(1., 1., 1.);
          } else {
            glColor3d(1., 0., 0.);
          }
          glVertex2d((x + 0.) * scal, (y + 0.) * scal);
          glVertex2d((x + 1.) * scal, (y + 0.) * scal);
          glVertex2d((x + 1.) * scal, (y + 1.) * scal);
          glVertex2d((x + 0.) * scal, (y + 1.) * scal);
        }
      }
      glEnd();
    }
  }
}
