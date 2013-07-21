
#include "args.h"
#include "core.h"
#include "debug.h"
#include "reporter.h"
#include "os.h"
#include "pak.h"
#include "perfbar.h"
#include "version.h"

#include "GL/glew.h"

#include "frame/frame.h"

DEFINE_bool(help, false, "Get help");
DEFINE_bool(development, false, "Development tools");
DEFINE_bool(permit_ogl1, false, "Permit OpenGL 1.0 renderer. Note this may crash in many fascinating manners or fail to render important parts of the game, this is strictly for local testing, here there be dragons");
DEFINE_bool(sound, true, "Enable sound");
DEFINE_bool(report_send, false, "Send error report");

namespace Glorp {

  void glErrorCheck() {
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
      CHECK(false, "OPENGL ERROR - %d", err);
    }
  }

  /*static*/ bool Core::Prestartup() {
    if (FLAGS_help) {
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
    
    if (FLAGS_report_send) {
      report();
      return true;
    }
    
    // placed after report() so we don't forkbomb anyone
    Allow911();
    
    // get our pak system up and running
    PakInit();
    
    return false;
  }

  Core::Core() :
        m_audioEnabled(false),
        m_L(0),
        m_luaCrashed(false),
        m_event_system_update_begin(LUA_NOREF),
        m_event_system_update_end(LUA_NOREF),
        m_event_system_mouse(LUA_NOREF),
        m_event_system_key_down(LUA_NOREF),
        m_event_system_key_up(LUA_NOREF),
        m_event_system_key_type(LUA_NOREF),
        m_event_system_key_repeat(LUA_NOREF),
        m_env(0)
  {
    if (FLAGS_sound) {
      // TODO: sound init here, set m_audioenabled to true if it worked
    }
    glewInit();
    l_init();
  }

  Core::~Core() {
    l_shutdown();
  }

  void Core::Input(const Frame::InputEvent &event) {
    PerfStack perf(0.5, 0.5, 0.5);

    if (FLAGS_development && event.GetMode() == Frame::InputEvent::MODE_KEYDOWN && event.GetKey() == Frame::Key::F12) {
      l_shutdown();
      l_init();
    } else if (m_L && !m_luaCrashed) {
      m_env->Input(event);

      if (event.GetMode() == Frame::InputEvent::MODE_KEYDOWN || event.GetMode() == Frame::InputEvent::MODE_KEYUP || event.GetMode() == Frame::InputEvent::MODE_KEYREPEAT) {
        int kevid;
        if (event.GetMode() == Frame::InputEvent::MODE_KEYDOWN) {
          kevid = m_event_system_key_down;
        } else if (event.GetMode() == Frame::InputEvent::MODE_KEYREPEAT) {
          kevid = m_event_system_key_repeat;
        } else if (event.GetMode() == Frame::InputEvent::MODE_KEYUP) {
          kevid = m_event_system_key_up;
        }

        lua_pushstring(m_L, Frame::Key::StringFromKey(event.GetKey()));
        l_callEvent(m_L, kevid, 1);
      } else if (event.GetMode() == Frame::InputEvent::MODE_TYPE) {
        lua_pushstring(m_L, event.GetType().c_str());
        l_callEvent(m_L, m_event_system_key_type, 1);
      }
    }
  }

  Core::UpdateResult Core::Update() {
    // I don't even know what we do here, honestly. Figure this out later when we have a more complex rendering path?
    return UR_RENDER;
  }
  
  // hacky frame limiter
  const float s_fpsTarget = 60.f;
  double s_lastSecond = 0;
  void Core::Render() {
    if (m_L && !m_luaCrashed) {
      l_callEvent(m_L, m_event_system_update_begin, 0);
      glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      {
        PerfStack perf(0.6, 0.2, 0.2);
        m_env->Render();
      }

      if (FLAGS_development) {
        perfbarDraw();

        // frame limiter to make our perfbar work better
        while (s_lastSecond + 1 / s_fpsTarget > timeMicro() / 1000000.);
        s_lastSecond = timeMicro() / 1000000.;
      }

      perfbarReset();
      l_callEvent(m_L, m_event_system_update_end, 0);
    } else {
      glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glScalef(2.0f / Version::gameXres, -2.0f / Version::gameYres, 1.0f);
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();

      glBegin(GL_QUADS);
      int scal = 50;
      for (int x = -5; x < 5; x++) {
        for (int y = -2; y < 2; y++) {
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
