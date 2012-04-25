
#include "os.h"

#include "core.h"
#include "debug.h"
#include "version.h"
#include "init.h"

#define _WIN32_IE 0x0500 // ffff
#define DIRECTINPUT_VERSION 0x0800

#include <windows.h>
#include <shlobj.h>
#include <psapi.h>
#include <excpt.h>
#include <imagehlp.h>
#include <direct.h>
#include <dinput.h>

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

  static int s_mouse_x = 0;
  static int s_mouse_y = 0;

  static HWND s_window = 0;

  static int s_width = 0;
  static int s_height = 0;

  static IDirectInput8A *s_di;
  static IDirectInputDevice8 *s_di_keyboard;
  static IDirectInputDevice8 *s_di_mouse;

  const int c_di_bufferSize = 64;
  const int c_bpp = 32;
  const int c_depth = 16;
  
  const Key c_keyIndex[] = {0,
    27, '1', '2', '3', '4',
    '5', '6', '7', '8', '9',
    '0', '-', '=', 8, 9,
    'q', 'w', 'e', 'r', 't',
    'y', 'u', 'i', 'o', 'p',
    '[', ']', 13, Keys::LeftControl, 'a',
    's', 'd', 'f', 'g', 'h',
    'j', 'k', 'l', ';', '\'',
    '`', Keys::LeftShift, '\\', 'z', 'x',
    'c', 'v', 'b', 'n', 'm',                                 // 50
    ',', '.', '/', Keys::RightShift, Keys::PadMultiply,
    Keys::LeftAlt, ' ', Keys::CapsLock, Keys::F1, Keys::F2,
    Keys::F3, Keys::F4, Keys::F5, Keys::F6, Keys::F7,
    Keys::F8, Keys::F9, Keys::F10, Keys::NumLock, Keys::ScrollLock,
    Keys::Pad7, Keys::Pad8, Keys::Pad9, Keys::PadSubtract, Keys::Pad4,
    Keys::Pad5, Keys::Pad6, Keys::PadAdd, Keys::Pad1, Keys::Pad2,
    Keys::Pad3, Keys::Pad0, Keys::PadDecimal, -1, -1,
    -1, Keys::F11, Keys::F12, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,                                      // 100
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,                                      // 150
    -1, -1, -1, -1, -1,
    Keys::PadEnter, Keys::RightControl, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    Keys::PadDivide, -1, Keys::PrintScreen, Keys::RightAlt, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, Keys::Pause, -1, Keys::Home, Keys::Up,                     // 200
    Keys::PageUp, -1, Keys::Left, -1, Keys::Right,
    -1, Keys::End, Keys::Down, Keys::PageDown, Keys::Insert,
    Keys::Delete, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,                                      // 250
    -1, -1, -1, -1, -1};

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
  
  // Handles window messages that arrive by any means, message queue or by direct notification.
  // However, key events are ignored, as input is handled by DirectInput in WindowThink().
  LRESULT CALLBACK HandleMessage(HWND window_handle, UINT message, WPARAM wparam, LPARAM lparam) {
    // Extract information from the parameters
    unsigned short wparam1 = LOWORD(wparam)/*, wparam2 = HIWORD(wparam)*/;
    unsigned short lparam1 = LOWORD(lparam), lparam2 = HIWORD(lparam);

  	// Handle each message
    switch (message) {
      case WM_SYSCOMMAND:
        // Prevent screen saver and monitor power saving
        if (wparam == SC_SCREENSAVE || wparam == SC_MONITORPOWER)
          return 0;
        // Prevent accidental pausing by pushing F10 or what not
        if (wparam == SC_MOUSEMENU || wparam == SC_KEYMENU)
          return 0;
        break;
      case WM_CLOSE:
        s_shutdown = true;
        return 0;
      case WM_MOVE:
        //os_window->x = (signed short)lparam1;
        //os_window->y = (signed short)lparam2;
        break;
      case WM_SIZE:
        // Set the resolution if a full-screen window was alt-tabbed into.
        if (s_minimized != (wparam == SIZE_MINIMIZED) && s_fullscreen) {
          if (wparam == SIZE_MINIMIZED) {
            ChangeDisplaySettings(0, 0);
          } else {
            DEVMODE screen_settings;
  	        screen_settings.dmSize = sizeof(screen_settings);
            screen_settings.dmDriverExtra = 0;
  	        screen_settings.dmPelsWidth = lparam1;
  	        screen_settings.dmPelsHeight = lparam2;
            screen_settings.dmBitsPerPel = c_bpp;
            screen_settings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
            ChangeDisplaySettings(&screen_settings, CDS_FULLSCREEN);
          }
        }
        s_minimized = (wparam == SIZE_MINIMIZED);
        if (!s_minimized) {
          s_width = lparam1;
          s_height = lparam2;
        }
        break;
      case WM_SIZING:
        break;
  	  case WM_ACTIVATE:
        s_focused = (wparam1 == WA_ACTIVE || wparam1 == WA_CLICKACTIVE);
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
    }

    // Pass on remaining messages to the default handler
    
    return DefWindowProcW(window_handle, message, wparam, lparam);
  }

  bool EventSort(const KeyEvent &lhs, const KeyEvent &rhs) {
    return lhs.timestamp < rhs.timestamp;
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

    // Attempt to initialize DirectInput.
    // Settings: Non-exclusive (be friendly with other programs), foreground (only accept input
    //                          events if we are currently in the foreground).
    if (FAILED(DirectInput8Create(GetModuleHandle(0), DIRECTINPUT_VERSION, IID_IDirectInput8A, (void**)&s_di, 0))) {
      CHECK(false);
      return 0;
    }
    s_di->CreateDevice(GUID_SysKeyboard, &s_di_keyboard, NULL);
    s_di_keyboard->SetCooperativeLevel(s_window, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
    s_di_keyboard->SetDataFormat(&c_dfDIKeyboard);
    s_di->CreateDevice(GUID_SysMouse, &s_di_mouse, NULL);
    s_di_mouse->SetCooperativeLevel(s_window, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
    s_di_mouse->SetDataFormat(&c_dfDIMouse2);

    // Set the DirectInput buffer size - this is the number of events it can store at a single time
    DIPROPDWORD prop_buffer_size;
    prop_buffer_size.diph.dwSize = sizeof(DIPROPDWORD);
    prop_buffer_size.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    prop_buffer_size.diph.dwObj = 0;
    prop_buffer_size.diph.dwHow = DIPH_DEVICE;
    prop_buffer_size.dwData = c_di_bufferSize;
    s_di_keyboard->SetProperty(DIPROP_BUFFERSIZE, &prop_buffer_size.diph);
    s_di_mouse->SetProperty(DIPROP_BUFFERSIZE, &prop_buffer_size.diph);

    {
      Core core;

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

        {
          vector<KeyEvent> events;

          // Hack: Insert the mouse position with GetCursorPos().
          // If we want to be super-accurate then we need to do some crazy synching work with GetCursorPos() and DX deltas
          // Right now, though, we really don't care about that.
          {
            POINT cursor_pos;
            GetCursorPos(&cursor_pos);
            ScreenToClient(s_window, &cursor_pos);

            if (s_mouse_x != cursor_pos.x || s_mouse_y != cursor_pos.y)
            {
              KeyEvent event;
              event.timestamp = 0;  // this is somewhat buggy
              event.pressed = false;
              event.mouse_x = cursor_pos.x;
              event.mouse_y = cursor_pos.y;
              event.alt = GetKeyState(VK_MENU);
              event.ctrl = GetKeyState(VK_CONTROL);
              event.shift = GetKeyState(VK_SHIFT);
              events.push_back(event);

              s_mouse_x = cursor_pos.x;
              s_mouse_y = cursor_pos.y;
            }
          }

          DIDEVICEOBJECTDATA buffer[c_di_bufferSize];
          unsigned long num_items = 0;
          HRESULT hr = 0;

          num_items = c_di_bufferSize;
          hr = s_di_keyboard->GetDeviceData(sizeof(buffer[0]), buffer, &num_items, 0);
          if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED) {
            s_di_keyboard->Acquire();
            hr = s_di_keyboard->GetDeviceData(sizeof(buffer[0]), buffer, &num_items, 0);
          }
          if (!FAILED(hr)) {
            for (int i = 0; i < (int)num_items; i++) {
              if (buffer[i].dwOfs < 255 && c_keyIndex[buffer[i].dwOfs] != -1) {
                KeyEvent event;
                event.timestamp = buffer[i].dwTimeStamp;
                event.pressed = buffer[i].dwData;
                event.key = c_keyIndex[buffer[i].dwOfs];
                event.alt = GetKeyState(VK_MENU);
                event.ctrl = GetKeyState(VK_CONTROL);
                event.shift = GetKeyState(VK_SHIFT);
                events.push_back(event);
              }
            }
          }

          num_items = c_di_bufferSize;
          hr = s_di_mouse->GetDeviceData(sizeof(buffer[0]), buffer, &num_items, 0);
          if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED) {
            s_di_mouse->Acquire();
            hr = s_di_mouse->GetDeviceData(sizeof(buffer[0]), buffer, &num_items, 0);
          }
          if (!FAILED(hr)) {
            for (int i = 0; i < (int)num_items; i++) {
              /*if (buffer[i].dwOfs == DIMOFS_X) {
                KeyEvent event;
                event.timestamp = buffer[i].dwTimeStamp;
                event.mouse_x = buffer[i].dwData;
                events.push_back(event);
              } else if (buffer[i].dwOfs == DIMOFS_Y) {
                KeyEvent event;
                event.timestamp = buffer[i].dwTimeStamp;
                event.mouse_y = buffer[i].dwData;
                events.push_back(event);
              } else*/ if (buffer[i].dwOfs == DIMOFS_BUTTON0) {
                KeyEvent event;
                event.timestamp = buffer[i].dwTimeStamp;
                event.key = Keys::MouseLButton;
                event.pressed = buffer[i].dwData;
                event.alt = GetKeyState(VK_MENU);
                event.ctrl = GetKeyState(VK_CONTROL);
                event.shift = GetKeyState(VK_SHIFT);
                events.push_back(event);
              } else if (buffer[i].dwOfs == DIMOFS_BUTTON1) {
                KeyEvent event;
                event.timestamp = buffer[i].dwTimeStamp;
                event.key = Keys::MouseRButton;
                event.pressed = buffer[i].dwData;
                event.alt = GetKeyState(VK_MENU);
                event.ctrl = GetKeyState(VK_CONTROL);
                event.shift = GetKeyState(VK_SHIFT);
                events.push_back(event);
              } else if (buffer[i].dwOfs == DIMOFS_BUTTON2) {
                KeyEvent event;
                event.timestamp = buffer[i].dwTimeStamp;
                event.key = Keys::MouseMButton;
                event.pressed = buffer[i].dwData;
                event.alt = GetKeyState(VK_MENU);
                event.ctrl = GetKeyState(VK_CONTROL);
                event.shift = GetKeyState(VK_SHIFT);
                events.push_back(event);
              } else if (buffer[i].dwOfs == DIMOFS_BUTTON3) {
                KeyEvent event;
                event.timestamp = buffer[i].dwTimeStamp;
                event.key = Keys::Mouse4Button;
                event.pressed = buffer[i].dwData;
                events.push_back(event);
              } else if (buffer[i].dwOfs == DIMOFS_BUTTON4) {
                KeyEvent event;
                event.timestamp = buffer[i].dwTimeStamp;
                event.key = Keys::Mouse5Button;
                event.pressed = buffer[i].dwData;
                event.alt = GetKeyState(VK_MENU);
                event.ctrl = GetKeyState(VK_CONTROL);
                event.shift = GetKeyState(VK_SHIFT);
                events.push_back(event);
              } else if (buffer[i].dwOfs == DIMOFS_BUTTON5) {
                KeyEvent event;
                event.timestamp = buffer[i].dwTimeStamp;
                event.key = Keys::Mouse6Button;
                event.pressed = buffer[i].dwData;
                event.alt = GetKeyState(VK_MENU);
                event.ctrl = GetKeyState(VK_CONTROL);
                event.shift = GetKeyState(VK_SHIFT);
                events.push_back(event);
              } else if (buffer[i].dwOfs == DIMOFS_BUTTON6) {
                KeyEvent event;
                event.timestamp = buffer[i].dwTimeStamp;
                event.key = Keys::Mouse7Button;
                event.pressed = buffer[i].dwData;
                event.alt = GetKeyState(VK_MENU);
                event.ctrl = GetKeyState(VK_CONTROL);
                event.shift = GetKeyState(VK_SHIFT);
                events.push_back(event);
              } else if (buffer[i].dwOfs == DIMOFS_BUTTON7) {
                KeyEvent event;
                event.timestamp = buffer[i].dwTimeStamp;
                event.key = Keys::Mouse8Button;
                event.pressed = buffer[i].dwData;
                event.alt = GetKeyState(VK_MENU);
                event.ctrl = GetKeyState(VK_CONTROL);
                event.shift = GetKeyState(VK_SHIFT);
                events.push_back(event);
              }
            }
          }

          // Okay, we've got our events. Next, sort:
          sort(events.begin(), events.end(), EventSort);

          // And now we want to go through and flesh out the mouse positions
          for (int i = 0; i < events.size(); ++i) {
            if (events[i].mouse_x != KeyEvent::MOUSEPOS_UNKNOWN)
              s_mouse_x = events[i].mouse_x;
            if (events[i].mouse_y != KeyEvent::MOUSEPOS_UNKNOWN)
              s_mouse_y = events[i].mouse_y;

            events[i].mouse_x = s_mouse_x;
            events[i].mouse_y = s_mouse_y;
          }

          // Finally, start plowing events into the core
          for (int i = 0; i < events.size(); ++i) {
            if (i + 1 != events.size() && events[i].timestamp == events[i + 1].timestamp && events[i].key == Keys::NoKey && events[i + 1].key == Keys::NoKey)
              continue; // this is a first of a mouse event pair, ignore it
            core.Event(events[i]);
          }
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
    }
    
    if (s_fullscreen && !s_minimized)
      ChangeDisplaySettings(NULL, 0);

    if (s_di_keyboard) {
      s_di_keyboard->Unacquire();
      s_di_keyboard->Release();
    }
    if (s_di_mouse) {
      s_di_mouse->Unacquire();
      s_di_mouse->Release();
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

