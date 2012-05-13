
#include "os.h"

#include "core.h"
#include "debug.h"
#include "version.h"
#include "init.h"

#define _WIN32_IE 0x0501
#define _WIN32_WINNT 0x0501

#include <windows.h>
#include <shlobj.h>
#include <psapi.h>
#include <excpt.h>
#include <imagehlp.h>
#include <direct.h>

#include <algorithm>

#include <GL/glew.h>

using namespace std;

namespace Glorp {
  static LARGE_INTEGER s_timerFrequency;
  static CONTEXT *s_crashContext = NULL;
  static bool s_minimized = false;
  static bool s_fullscreen = false;
  static bool s_focused = false;
  static bool s_shutdown = false;

  static HWND s_window = 0;

  static int s_width = 0;
  static int s_height = 0;

  static Core *s_core = 0;

  const int c_bpp = 32;
  const int c_depth = 16;
  
  const Key c_keyIndex[] = {
    -1, Keys::MouseLButton, Keys::MouseRButton, -1, Keys::MouseMButton, Keys::Mouse4Button, Keys::Mouse5Button, -1, // 0x00 - 0x07
    Keys::Backspace, Keys::Tab, -1, -1, -1, Keys::Enter, -1, -1, // 0x08 - 0x0f
    -1, -1, -1, Keys::Pause, Keys::CapsLock, -1, -1, -1, // 0x10 - 0x17
    -1, -1, -1, -1, Keys::Escape, -1, -1, -1, // 0x18 - 0x1f
    ' ', Keys::PageUp, Keys::PageDown, Keys::End, Keys::Home, Keys::Left, Keys::Up, Keys::Right, // 0x20 - 0x27
    Keys::Down, -1, -1, -1, Keys::PrintScreen, Keys::Insert, Keys::Delete, -1, // 0x28 - 0x2f
    '0', '1', '2', '3', '4', '5', '6', '7', // 0x30 - 0x37
    '8', '9', -1, -1, -1, -1, -1, -1, // 0x38 - 0x3f
    -1, 'a', 'b', 'c', 'd', 'e', 'f', 'g', // 0x40 - 0x47
    'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', // 0x48 - 0x4f
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', // 0x50 - 0x57
    'x', 'y', 'z', Keys::LeftGui, Keys::RightGui, -1, -1, -1, // 0x58 - 0x5f
    Keys::Pad0, Keys::Pad1, Keys::Pad2, Keys::Pad3, Keys::Pad4, Keys::Pad5, Keys::Pad6, Keys::Pad7,  // 0x60 - 0x67
    Keys::Pad8, Keys::Pad9, Keys::PadMultiply, Keys::PadAdd, -1, Keys::PadSubtract, Keys::PadDecimal, Keys::PadDivide, // 0x68 - 0x6f
    Keys::F1, Keys::F2, Keys::F3, Keys::F4, Keys::F5, Keys::F6, Keys::F7, Keys::F8, // 0x70 - 0x77
    Keys::F9, Keys::F10, Keys::F11, Keys::F12, Keys::F13, Keys::F14, Keys::F15, Keys::F16, // 0x78 - 0x7f
    Keys::F17, Keys::F18, Keys::F19, Keys::F20, Keys::F21, Keys::F22, Keys::F23, Keys::F24, // 0x80 - 0x87
    -1, -1, -1, -1, -1, -1, -1, -1, // 0x88 - 0x8f
    Keys::NumLock, Keys::ScrollLock, -1, -1, -1, -1, -1, -1, // 0x90 - 0x97
    -1, -1, -1, -1, -1, -1, -1, -1, // 0x98 - 0x9f
    Keys::LeftShift, Keys::RightShift, Keys::LeftControl, Keys::RightControl, Keys::LeftMenu, Keys::RightMenu, -1, -1, // 0xa0 - 0xa7
    -1, -1, -1, -1, -1, -1, -1, -1, // 0xa8 - 0xaf
    -1, -1, -1, -1, -1, -1, -1, -1, // 0xb0 - 0xb7
    -1, -1, ';', '=', ',', '-', '.', '/', // 0xb8 - 0xbf
    '`', -1, -1, -1, -1, -1, -1, -1, // 0xc0 - 0xc7
    -1, -1, -1, -1, -1, -1, -1, -1, // 0xc8 - 0xcf
    -1, -1, -1, -1, -1, -1, -1, -1, // 0xd0 - 0xd7
    -1, -1, -1, '[', '\\', ']', '\'', -1, // 0xd8 - 0xdf
    -1, -1, -1, -1, -1, -1, -1, -1, // 0xe0 - 0xe7
    -1, -1, -1, -1, -1, -1, -1, -1, // 0xe8 - 0xef
    -1, -1, -1, -1, -1, -1, -1, -1, // 0xf0 - 0xf7
    -1, -1, -1, -1, -1, -1, -1, -1, // 0xf8 - 0xff
  };

  void outputDebug(const string &str) {
    OutputDebugString(str.c_str());
  }
  
  void crash() {
    TerminateProcess(GetCurrentProcess(), 1); // sigh. this should not be necessary. on cygwin, sometimes it is. fuck cygwin.
    exit(-1);
  }
  
  string directoryDesktop() {
    char meep[MAX_PATH + 1] = {0};
    SHGetFolderPath(NULL, CSIDL_DESKTOPDIRECTORY, NULL, SHGFP_TYPE_CURRENT, meep);
    return meep;
  }
  
  void spawn(const string &exec, const vector<string> &params) {
    string texec = exec;

    for(int i = 0; i < params.size(); i++) {
      CHECK(count(params[i].begin(), params[i].end(), '"') == 0);
      texec += " \"" + params[i] + "\"";
    }

    STARTUPINFO sinfo;
    memset(&sinfo, 0, sizeof(sinfo));
    sinfo.cb = sizeof(sinfo);
    
    PROCESS_INFORMATION pi;
    CHECK(CreateProcess(NULL, (char*)texec.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &sinfo, &pi));
    CloseHandle(&pi);
  }

  void beginShutdown() {
    s_shutdown = true;
  }
  
  class SignalHandler {
  public:
    static WINAPI LONG signal_h(EXCEPTION_POINTERS *lol) {
      s_crashContext = lol->ContextRecord;
      CHECK(0);
      crash();
    } // WE NEVER DIE
    
    SignalHandler() {
      SetUnhandledExceptionFilter(&signal_h);
    }
  } sighandler;

  int memoryUsage() {
    PROCESS_MEMORY_COUNTERS pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
    return pmc.WorkingSetSize;
  }

  int64_t timeMicro() {
    LARGE_INTEGER current_time;
    QueryPerformanceCounter(&current_time);
    return int64_t((1000000 * (long double)current_time.QuadPart) / s_timerFrequency.QuadPart);
    // on my windows box, the timer frequency is about 2.4 billion (clock speed ahoy). That gives one overflow every 2 hours if done with the same method GetTime() uses, and without floating-point. On a faster system that might go down to 1 hour. Unacceptable. We go to floating-point to avoid issues of this sort. Dividing by 1000 might do the job, but I'm unsure how *low* TimerFrequency might go. It's all kind of nasty.
  }
  
  void stackDump(vector<const void*> *data) {
    CONTEXT *context = s_crashContext;
    
    CONTEXT ctx;
    if(!context) {
      // oh bloody hell
      HINSTANCE kernel32 = LoadLibrary("Kernel32.dll");
      typedef void ( * RtlCaptureContextFunc ) ( CONTEXT * ContextRecord );
      RtlCaptureContextFunc rtlCaptureContext = (RtlCaptureContextFunc) GetProcAddress( kernel32, "RtlCaptureContext" );
      rtlCaptureContext(&ctx);
      context = &ctx;
    }

    STACKFRAME frame;
    memset(&frame, 0, sizeof(frame));

    frame.AddrPC.Offset = context->Eip;
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrStack.Offset = context->Esp;
    frame.AddrStack.Mode = AddrModeFlat;
    frame.AddrFrame.Offset = context->Ebp;
    frame.AddrFrame.Mode = AddrModeFlat;

    while(StackWalk(IMAGE_FILE_MACHINE_I386, GetCurrentProcess(), GetCurrentThread(), &frame, context, 0, SymFunctionTableAccess, SymGetModuleBase, 0)) {
      data->push_back((const void*)frame.AddrPC.Offset);
    }
  }
  
  static const string directory_delimiter = "\\";

  string directoryConfig() {
    char buff[MAX_PATH + 1];
    SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, buff);
    //dprintf("Home directory: %s\n", buff);
    return string(buff) + "\\" + Version::gameFullname + "\\";
  }

  void directoryMkdir(const string &str) {
    _mkdir(str.c_str());
  }

  string directoryTempfile() {
    char buff[MAX_PATH + 1];
    GetTempPath(sizeof(buff), buff);
    char fname[MAX_PATH + 1];
    GetTempFileName(buff, Version::gameSlug, 0, fname);
    return fname;
  }
  
  string directoryDelimiter() {
    return "\\";
  }

  KeyEvent CreateBasicEvent() {
    KeyEvent rv;
    POINT mouse;
    GetCursorPos(&mouse);
    ScreenToClient(s_window, &mouse);
    rv.mouse_x = mouse.x;
    rv.mouse_y = mouse.y;
    rv.shift = GetKeyState(VK_SHIFT) & 0x80;
    rv.ctrl = GetKeyState(VK_CONTROL) & 0x80;
    rv.alt = GetKeyState(VK_MENU) & 0x80;
    return rv;
  }
  
  // Handles window messages that arrive by any means, message queue or by direct notification.
  // However, key events are ignored, as input is handled by DirectInput in WindowThink().
  LRESULT CALLBACK HandleMessage(HWND window_handle, UINT message, WPARAM wParam, LPARAM lParam) {
    // Extract information from the parameters
    unsigned short wParam1 = LOWORD(wParam)/*, wParam2 = HIWORD(wParam)*/;
    unsigned short lParam1 = LOWORD(lParam), lParam2 = HIWORD(lParam);

  	// Handle each message
    switch (message) {
      case WM_SYSCOMMAND:
        // Prevent screen saver and monitor power saving
        if (wParam == SC_SCREENSAVE || wParam == SC_MONITORPOWER)
          return 0;
        // Prevent accidental pausing by pushing F10 or what not
        if (wParam == SC_MOUSEMENU || wParam == SC_KEYMENU)
          return 0;
        break;
      case WM_CLOSE:
        s_shutdown = true;
        return 0;
      case WM_MOVE:
        //os_window->x = (signed short)lParam1;
        //os_window->y = (signed short)lParam2;
        break;
      case WM_SIZE:
        // Set the resolution if a full-screen window was alt-tabbed into.
        if (s_minimized != (wParam == SIZE_MINIMIZED) && s_fullscreen) {
          if (wParam == SIZE_MINIMIZED) {
            ChangeDisplaySettings(0, 0);
          } else {
            DEVMODE screen_settings;
  	        screen_settings.dmSize = sizeof(screen_settings);
            screen_settings.dmDriverExtra = 0;
  	        screen_settings.dmPelsWidth = lParam1;
  	        screen_settings.dmPelsHeight = lParam2;
            screen_settings.dmBitsPerPel = c_bpp;
            screen_settings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
            ChangeDisplaySettings(&screen_settings, CDS_FULLSCREEN);
          }
        }
        s_minimized = (wParam == SIZE_MINIMIZED);
        if (!s_minimized) {
          s_width = lParam1;
          s_height = lParam2;
        }
        break;
      case WM_SIZING:
        break;
  	  case WM_ACTIVATE:
        s_focused = (wParam1 == WA_ACTIVE || wParam1 == WA_CLICKACTIVE);
        // If the user alt-tabs out of a fullscreen window, the window will keep drawing and will
        // remain in full-screen mode. Here, we minimize the window, which fixes the drawing problem,
        // and then the WM_SIZE event fixes the full-screen problem.
        if (!s_focused && s_fullscreen)
          ShowWindow(s_window, SW_MINIMIZE);
        /*if (s_focused && s_cursorlocked)
          LockCursorNow(os_window);
        if (!s_focused)
          UnlockCursorNow();*/
        break;
      case WM_KEYDOWN:
      case WM_SYSKEYDOWN:
        if (s_core && wParam > 0 && wParam < 256) {
          KeyEvent event = CreateBasicEvent();
          if (lParam & (1 << 30)) {
            event.pressed = KeyEvent::REPEAT;
          } else {
            event.pressed = KeyEvent::DOWN;
          }
          event.key = c_keyIndex[wParam];
          s_core->Event(event);
          // TODO: support built-in key repeat, maybe?
        }
        break;
      case WM_KEYUP:
      case WM_SYSKEYUP:
        if (s_core && wParam > 0 && wParam < 256) {
          KeyEvent event = CreateBasicEvent();
          event.pressed = KeyEvent::UP;
          event.key = c_keyIndex[wParam];
          s_core->Event(event);
        }
        break;
      case WM_UNICHAR:
        if (s_core && wParam != UNICODE_NOCHAR) {
          KeyEvent event = CreateBasicEvent();
          event.typed += (char)wParam; // TODO: utf8ize
          s_core->Event(event);
        }
        break;
      case WM_CHAR:
        if (s_core && wParam != '\t' && wParam != '\r' && wParam != '\b' && wParam != '\033') { // windows passes a bunch of nonprintable characters through this way, thanks windows. thwindows.
          KeyEvent event = CreateBasicEvent();
          event.typed += (char)wParam; // TODO: utf8ize
          s_core->Event(event);
        }
        break;
      case WM_MOUSEMOVE:
        if (s_core) {
          s_core->Event(CreateBasicEvent()); // this automatically wraps up the current mouse position
        }
        break;
      case WM_LBUTTONDOWN:
        if (s_core) {
          KeyEvent event = CreateBasicEvent();
          event.pressed = KeyEvent::DOWN;
          event.key = Keys::MouseLButton;
          s_core->Event(event);
        }
        break;
      case WM_LBUTTONUP:
        if (s_core) {
          KeyEvent event = CreateBasicEvent();
          event.pressed = KeyEvent::UP;
          event.key = Keys::MouseLButton;
          s_core->Event(event);
        }
        break;
      case WM_MBUTTONDOWN:
        if (s_core) {
          KeyEvent event = CreateBasicEvent();
          event.pressed = KeyEvent::DOWN;
          event.key = Keys::MouseMButton;
          s_core->Event(event);
        }
        break;
      case WM_MBUTTONUP:
        if (s_core) {
          KeyEvent event = CreateBasicEvent();
          event.pressed = KeyEvent::UP;
          event.key = Keys::MouseMButton;
          s_core->Event(event);
        }
        break;
      case WM_RBUTTONDOWN:
        if (s_core) {
          KeyEvent event = CreateBasicEvent();
          event.pressed = KeyEvent::DOWN;
          event.key = Keys::MouseRButton;
          s_core->Event(event);
        }
        break;
      case WM_RBUTTONUP:
        if (s_core) {
          KeyEvent event = CreateBasicEvent();
          event.pressed = KeyEvent::UP;
          event.key = Keys::MouseRButton;
          s_core->Event(event);
        }
        break;
      case WM_XBUTTONDOWN:
        if (s_core) {
          KeyEvent event = CreateBasicEvent();
          event.pressed = KeyEvent::DOWN;
          if ((wParam >> 16) == XBUTTON1)
          {
            event.key = Keys::Mouse4Button;
          }
          else if ((wParam >> 16) == XBUTTON2)
          {
            event.key = Keys::Mouse5Button;
          }
          s_core->Event(event);
        }
        break;
      case WM_XBUTTONUP:
        if (s_core) {
          KeyEvent event = CreateBasicEvent();
          event.pressed = KeyEvent::UP;
          if ((wParam >> 16) == XBUTTON1)
          {
            event.key = Keys::Mouse4Button;
          }
          else if ((wParam >> 16) == XBUTTON2)
          {
            event.key = Keys::Mouse5Button;
          }
          s_core->Event(event);
        }
        break;
    }

    // Pass on remaining messages to the default handler
    
    return DefWindowProcW(window_handle, message, wParam, lParam);
  }

  int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
  {
    dprintf("-------------------------------------------------------------------------------------");

    // let's get this thing started
    initProgram(&__argc, const_cast<const char ***>(&__argv));

    if (Core::Prestartup())
    {
      return 0;
    }

    QueryPerformanceFrequency(&s_timerFrequency);

    const wchar_t *const kClassName = L"GlorpWin32";

    WNDCLASSW window_class;
    window_class.style = CS_OWNDC;
    window_class.lpfnWndProc = HandleMessage;
    window_class.cbClsExtra = 0;
    window_class.cbWndExtra = 0;
    window_class.hInstance = GetModuleHandle(0);
    window_class.hIcon = 0;
    window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    window_class.hbrBackground = 0;
    window_class.lpszMenuName = 0;
    window_class.lpszClassName = kClassName;
    if (!RegisterClassW(&window_class))
    {
      CHECK(false);
      return 0;
    }

    DWORD window_style = WS_POPUP;
    if (!s_fullscreen) {
      window_style = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
      if (Version::resizable)
        window_style |= WS_MAXIMIZEBOX | WS_THICKFRAME;
    }

    DWORD window_style_ex = 0;

    // Specify the window dimensions and get the border size
    RECT window_rectangle;
    window_rectangle.left = 0;
    window_rectangle.right = Version::gameXres;
    window_rectangle.top = 0;
    window_rectangle.bottom = Version::gameYres;
    if (!AdjustWindowRectEx(&window_rectangle, window_style, false, window_style_ex))
    {
      CHECK(false);
      return 0;
    }

    s_window = CreateWindowExW(window_style_ex, kClassName, L"Glop window", window_style, CW_USEDEFAULT, CW_USEDEFAULT, window_rectangle.right - window_rectangle.left, window_rectangle.bottom - window_rectangle.top, NULL, NULL, GetModuleHandle(0), NULL);

    if (!s_window)
    {
      CHECK(s_window);
      return 0;
    }

    SetWindowText(s_window, Version::gameFullname);

    // Set icon
    SendMessage(s_window, WM_SETICON, ICON_BIG, (LPARAM)LoadImage(GetModuleHandle(0), MAKEINTRESOURCE(101), IMAGE_ICON, 32, 32, 0));
    SendMessage(s_window, WM_SETICON, ICON_SMALL, (LPARAM)LoadImage(GetModuleHandle(0), MAKEINTRESOURCE(101), IMAGE_ICON, 16, 16, 0));

    // Get the window position
    RECT actual_position;
    GetWindowRect(s_window, &actual_position);
    s_width = actual_position.right - actual_position.left;
    s_height = actual_position.bottom - actual_position.top;

    // Get the device context
    HDC deviceContext = GetDC(s_window);
    if (!deviceContext) {
      CHECK(false);
      return 0;
    }

    // Specify a pixel format by requesting one, and then selecting the best available match on the
    // system. This is used to set up Open Gl.
    PIXELFORMATDESCRIPTOR pixel_format_request;
    memset(&pixel_format_request, 0, sizeof(pixel_format_request));
    pixel_format_request.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pixel_format_request.nVersion = 1;
    pixel_format_request.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pixel_format_request.iPixelType = PFD_TYPE_RGBA;
    pixel_format_request.cColorBits = c_bpp;
    pixel_format_request.cStencilBits = Version::stencil;
    pixel_format_request.cDepthBits = c_depth;
    unsigned int pixel_format_id = ChoosePixelFormat(deviceContext, &pixel_format_request);
    if (!pixel_format_id) {
      CHECK(false);
      return 0;
    }
    if (!SetPixelFormat(deviceContext, pixel_format_id, &pixel_format_request)) {
      CHECK(false);
      return 0;
    }

      // Switch to full-screen mode if appropriate
    if (s_fullscreen) {
      DEVMODE screen_settings;
  	  screen_settings.dmSize = sizeof(screen_settings);
      screen_settings.dmDriverExtra = 0;
      screen_settings.dmPelsWidth = s_width;
  	  screen_settings.dmPelsHeight = s_height;
      screen_settings.dmBitsPerPel = c_bpp;
      screen_settings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
      if (ChangeDisplaySettings(&screen_settings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
        CHECK(false);
        return 0;
      }
    }

    // Make a rendering context for this thread
    HGLRC glContext = wglCreateContext(deviceContext);
    if (!glContext) {
      CHECK(false);
      return 0;
    }
    wglMakeCurrent(deviceContext, glContext);


    // turn vsync on
    {
      typedef BOOL (APIENTRY *WGLSwapProc)( int );
      WGLSwapProc sSwapProc = 0;
    
      const char *extensions = (const char*)glGetString(GL_EXTENSIONS);
      if (strstr(extensions, "WGL_EXT_swap_control") != 0)
        sSwapProc = (WGLSwapProc)wglGetProcAddress("wglSwapIntervalEXT");

      if (sSwapProc)
        sSwapProc(1);
    }

    // Show the window. Note that SetForegroundWindow can fail if the user is currently using another
    // window, but this is fine and nothing to be alarmed about.
    ShowWindow(s_window, SW_SHOW);
    SetForegroundWindow(s_window);
    SetFocus(s_window);

    {
      Core core;
      s_core = &core;

      // MAIN LOOP HERE
      while (true) {
        MSG message;
        while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
          TranslateMessage(&message);
          DispatchMessage(&message);
        }

        if (s_shutdown) {
          break;
        }

        Core::UpdateResult result = core.Update();

        if (result == Core::UR_RENDER) {
          core.Render();
          SwapBuffers(deviceContext);
        } else if (result == Core::UR_QUIT) {
          break;
        } else if (result == Core::UR_TICK) {
          
        } else {
          CHECK(0);
        }
      }

      s_core = NULL;
    }
    
    if (s_fullscreen && !s_minimized) {
      ChangeDisplaySettings(NULL, 0);
    }

    if (glContext) {
      wglDeleteContext(glContext);
    }
    if (deviceContext) {
      ReleaseDC(s_window, deviceContext);
    }
    if (s_window) {
      DestroyWindow(s_window);
    }

    // and we're done
    return 0;
  }
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  return Glorp::WinMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}

