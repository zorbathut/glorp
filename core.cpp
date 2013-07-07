
#include "args.h"
#include "core.h"
#include "debug.h"
#include "os.h"
#include "perfbar.h"
#include "version.h"

#include "GL/glew.h"

#include "frame/frame.h"

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
        m_event_system_key_down(LUA_NOREF),
        m_event_system_key_up(LUA_NOREF),
        m_event_system_key_type(LUA_NOREF),
        m_event_system_key_repeat(LUA_NOREF),
        m_env(0)
  {
    if (FLAGS_sound) {
      m_alcDevice = alcOpenDevice(NULL);
      if (m_alcDevice) {
        m_alcContext = alcCreateContext(m_alcDevice, NULL);
        CHECK(m_alcContext);
        if (m_alcContext) {
          alcMakeContextCurrent(m_alcContext);
          CHECK(alGetError() == AL_NO_ERROR);
          
          m_audioEnabled = true;
        }
      }
    }
    glewInit();
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
    PerfStack perf(0.5, 0.5, 0.5);

    if (FLAGS_development && event.key == Keys::F12 && event.pressed == KeyEvent::DOWN) {
      l_shutdown();
      l_init();
    } else if (m_L && !m_luaCrashed) {
      m_env->MouseMove(event.mouse_x, event.mouse_y);

      // mouse buttons!
      {
        int mid = -1;
        if (event.key == Keys::MouseLButton) {
          mid = 0;
        } else if(event.key == Keys::MouseRButton) {
          mid = 1;
        } else if(event.key == Keys::MouseMButton) {
          mid = 2;
        } else if(event.key == Keys::Mouse4Button) {
          mid = 3;
        } else if(event.key == Keys::Mouse5Button) {
          mid = 4;
        }
        
        if (mid != -1) {
          if (event.pressed == KeyEvent::DOWN) {
            m_env->MouseDown(mid);
          } else {
            m_env->MouseUp(mid);
          }
        }
      }

      {
        Frame::KeyEvent kev;

        if (event.key == 'a') kev.key = Frame::Key::A;
        if (event.key == 'b') kev.key = Frame::Key::B;
        if (event.key == 'c') kev.key = Frame::Key::C;
        if (event.key == 'd') kev.key = Frame::Key::D;
        if (event.key == 'e') kev.key = Frame::Key::E;
        if (event.key == 'f') kev.key = Frame::Key::F;
        if (event.key == 'g') kev.key = Frame::Key::G;
        if (event.key == 'h') kev.key = Frame::Key::H;
        if (event.key == 'i') kev.key = Frame::Key::I;
        if (event.key == 'j') kev.key = Frame::Key::J;
        if (event.key == 'k') kev.key = Frame::Key::K;
        if (event.key == 'l') kev.key = Frame::Key::L;
        if (event.key == 'm') kev.key = Frame::Key::M;
        if (event.key == 'n') kev.key = Frame::Key::N;
        if (event.key == 'o') kev.key = Frame::Key::O;
        if (event.key == 'p') kev.key = Frame::Key::P;
        if (event.key == 'q') kev.key = Frame::Key::Q;
        if (event.key == 'r') kev.key = Frame::Key::R;
        if (event.key == 's') kev.key = Frame::Key::S;
        if (event.key == 't') kev.key = Frame::Key::T;
        if (event.key == 'u') kev.key = Frame::Key::U;
        if (event.key == 'v') kev.key = Frame::Key::V;
        if (event.key == 'w') kev.key = Frame::Key::W;
        if (event.key == 'x') kev.key = Frame::Key::X;
        if (event.key == 'y') kev.key = Frame::Key::Y;
        if (event.key == 'z') kev.key = Frame::Key::Z;
        if (event.key == '0') kev.key = Frame::Key::Num0;
        if (event.key == '1') kev.key = Frame::Key::Num1;
        if (event.key == '2') kev.key = Frame::Key::Num2;
        if (event.key == '3') kev.key = Frame::Key::Num3;
        if (event.key == '4') kev.key = Frame::Key::Num4;
        if (event.key == '5') kev.key = Frame::Key::Num5;
        if (event.key == '6') kev.key = Frame::Key::Num6;
        if (event.key == '7') kev.key = Frame::Key::Num7;
        if (event.key == '8') kev.key = Frame::Key::Num8;
        if (event.key == '9') kev.key = Frame::Key::Num9;
        if (event.key == Keys::Escape) kev.key = Frame::Key::Escape;
        if (event.key == Keys::LeftControl) kev.key = Frame::Key::ControlLeft;
        if (event.key == Keys::LeftShift) kev.key = Frame::Key::ShiftLeft;
        if (event.key == Keys::LeftAlt) kev.key = Frame::Key::AltLeft;
        if (event.key == Keys::LeftGui) kev.key = Frame::Key::SystemLeft;
        if (event.key == Keys::RightControl) kev.key = Frame::Key::ControlRight;
        if (event.key == Keys::RightShift) kev.key = Frame::Key::ShiftRight;
        if (event.key == Keys::RightAlt) kev.key = Frame::Key::AltRight;
        if (event.key == Keys::RightGui) kev.key = Frame::Key::SystemRight;
        //if (event.key == Keys::) kev.key = Frame::Key::Menu;
        if (event.key == '[') kev.key = Frame::Key::BracketLeft;
        if (event.key == ']') kev.key = Frame::Key::BracketRight;
        if (event.key == ';') kev.key = Frame::Key::Semicolon;
        if (event.key == ',') kev.key = Frame::Key::Comma;
        if (event.key == '.') kev.key = Frame::Key::Period;
        if (event.key == '\'') kev.key = Frame::Key::Quote;
        if (event.key == '/') kev.key = Frame::Key::Slash;
        if (event.key == '\\') kev.key = Frame::Key::Backslash;
        if (event.key == '`') kev.key = Frame::Key::Tilde;
        if (event.key == '=') kev.key = Frame::Key::Equal;
        if (event.key == '-') kev.key = Frame::Key::Dash;
        if (event.key == ' ') kev.key = Frame::Key::Space;
        if (event.key == '\n') kev.key = Frame::Key::Return;
        if (event.key == Keys::Backspace) kev.key = Frame::Key::Backspace;
        if (event.key == '\t') kev.key = Frame::Key::Tab;
        if (event.key == Keys::PageUp) kev.key = Frame::Key::PageUp;
        if (event.key == Keys::PageDown) kev.key = Frame::Key::PageDown;
        if (event.key == Keys::End) kev.key = Frame::Key::End;
        if (event.key == Keys::Home) kev.key = Frame::Key::Home;
        if (event.key == Keys::Insert) kev.key = Frame::Key::Insert;
        if (event.key == Keys::Delete) kev.key = Frame::Key::Delete;
        if (event.key == Keys::PadAdd) kev.key = Frame::Key::Add;
        if (event.key == Keys::PadSubtract) kev.key = Frame::Key::Subtract;
        if (event.key == Keys::PadMultiply) kev.key = Frame::Key::Multiply;
        if (event.key == Keys::PadDivide) kev.key = Frame::Key::Divide;
        if (event.key == Keys::Left) kev.key = Frame::Key::Left;
        if (event.key == Keys::Right) kev.key = Frame::Key::Right;
        if (event.key == Keys::Up) kev.key = Frame::Key::Up;
        if (event.key == Keys::Down) kev.key = Frame::Key::Down;
        if (event.key == Keys::Pad0) kev.key = Frame::Key::Numpad0;
        if (event.key == Keys::Pad1) kev.key = Frame::Key::Numpad1;
        if (event.key == Keys::Pad2) kev.key = Frame::Key::Numpad2;
        if (event.key == Keys::Pad3) kev.key = Frame::Key::Numpad3;
        if (event.key == Keys::Pad4) kev.key = Frame::Key::Numpad4;
        if (event.key == Keys::Pad5) kev.key = Frame::Key::Numpad5;
        if (event.key == Keys::Pad6) kev.key = Frame::Key::Numpad6;
        if (event.key == Keys::Pad7) kev.key = Frame::Key::Numpad7;
        if (event.key == Keys::Pad8) kev.key = Frame::Key::Numpad8;
        if (event.key == Keys::Pad9) kev.key = Frame::Key::Numpad9;
        if (event.key == Keys::F1) kev.key = Frame::Key::F1;
        if (event.key == Keys::F2) kev.key = Frame::Key::F2;
        if (event.key == Keys::F3) kev.key = Frame::Key::F3;
        if (event.key == Keys::F4) kev.key = Frame::Key::F4;
        if (event.key == Keys::F5) kev.key = Frame::Key::F5;
        if (event.key == Keys::F6) kev.key = Frame::Key::F6;
        if (event.key == Keys::F7) kev.key = Frame::Key::F7;
        if (event.key == Keys::F8) kev.key = Frame::Key::F8;
        if (event.key == Keys::F9) kev.key = Frame::Key::F9;
        if (event.key == Keys::F10) kev.key = Frame::Key::F10;
        if (event.key == Keys::F11) kev.key = Frame::Key::F11;
        if (event.key == Keys::F12) kev.key = Frame::Key::F12;
        //if (event.key == Keys::Escape) kev.key = Frame::Key::F13;
        //if (event.key == Keys::Escape) kev.key = Frame::Key::F14;
        //if (event.key == Keys::Escape) kev.key = Frame::Key::F15;
        //if (event.key == Keys::Escape) kev.key = Frame::Key::Pause;

        if (kev.key != Frame::Key::INVALID) {
          kev.alt = event.alt;
          kev.ctrl = event.ctrl;
          kev.shift = event.shift;

          int kevid;
          if (event.pressed == KeyEvent::DOWN) {
            m_env->KeyDown(kev);
            kevid = m_event_system_key_down;
          } else if (event.pressed == KeyEvent::REPEAT) {
            m_env->KeyRepeat(kev);
            kevid = m_event_system_key_repeat;
          } else {
            m_env->KeyUp(kev);
            kevid = m_event_system_key_up;
          }

          lua_pushstring(m_L, Frame::Key::StringFromKey(kev.key));
          l_callEvent(m_L, kevid, 1);
        }
      }

      if (!event.typed.empty()) {
        m_env->KeyType(event.typed);

        lua_pushstring(m_L, event.typed.c_str());
        l_callEvent(m_L, m_event_system_key_type, 1);
      }

      // TODO: mousewheel
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
