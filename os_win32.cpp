
#include "os.h"
#include "debug.h"
#include "version.h"
#include "core.h"

#define _WIN32_IE 0x0500 // ffff

#include <windows.h>
#include <shlobj.h>
#include <psapi.h>
#include <excpt.h>
#include <imagehlp.h>
#include <direct.h>
#include <dinput.h>

#include <algorithm>

using namespace std;

namespace Glorp {
  static LARGE_INTEGER s_timerFrequency;
  static CONTEXT *s_crashContext = NULL;
  static bool s_minimized = false;
  static bool s_fullscreen = false;
  static bool s_focused = false;
  static bool s_shutdown = false;

  HWND s_window = 0;

  static int s_width = 0;
  static int s_height = 0;

  static IDirectInput8A *s_di;
  static IDirectInputDevice8 *s_di_keyboard;
  static IDirectInputDevice8 *s_di_mouse;

  const int c_di_bufferSize = 64;
  const int c_bpp = 32;
  const int c_depth = 16;
  
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
  
  vector<const void*> stackDump() {
    vector<const void*> stack;
    
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
      stack.push_back((const void*)frame.AddrPC.Offset);
    }

    return stack;
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
    unsigned short wparam1 = LOWORD(wparam), wparam2 = HIWORD(wparam);
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

  int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
  {
    // let's get this thing started
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

    // Specify the window dimensions and get the border size
    RECT window_rectangle;
    window_rectangle.left = 0;
    window_rectangle.right = Version::gameXres;
    window_rectangle.top = 0;
    window_rectangle.bottom = Version::gameYres;
    if (!AdjustWindowRectEx(&window_rectangle, window_style, false, window_style))
    {
      CHECK(false);
      return 0;
    }

    s_window = CreateWindowExW(0, kClassName, L"Glop window", window_style, CW_USEDEFAULT, CW_USEDEFAULT, window_rectangle.right - window_rectangle.left, window_rectangle.bottom - window_rectangle.top, NULL, NULL, GetModuleHandle(0), NULL);

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

    {
      Core core;

      while (true) {
        MSG message;
        while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
          TranslateMessage(&message);
          DispatchMessage(&message);
        }

        if (s_shutdown)
        {
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

