
#define _WIN32_IE 0x0501
#define _WIN32_WINNT 0x0501

#include "os.h"

#include "args.h"
#include "core.h"
#include "debug.h"
#include "version.h"
#include "init.h"

#include "frame/os_win32.h"

#include <windows.h>
#include <shlobj.h>
#include <psapi.h>
#include <excpt.h>
#include <imagehlp.h>
#include <direct.h>

#include <algorithm>

#include <GL/glew.h>

using namespace std;

DECLARE_bool(development);

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
  
  // used for aspect ratio enforce
  int g_window_adjust_x = 0;
  int g_window_adjust_y = 0;

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

  void resize(int edge, RECT *rect)
  {
    int size_x_desired = (rect->right - rect->left) - g_window_adjust_x;
    int size_y_desired = (rect->bottom - rect->top) - g_window_adjust_y;

    switch (edge)
    {
    case WMSZ_BOTTOM:
    case WMSZ_TOP:
      {
        int size_x = g_window_adjust_x + (size_y_desired * Version::gameXres) / Version::gameYres;
        rect->left = (rect->left + rect->right) / 2 - size_x / 2;
        rect->right = rect->left + size_x;
      }
      break;
    case WMSZ_BOTTOMLEFT:
      {
        int size_x, size_y;

        if (size_x_desired * Version::gameYres > size_y_desired * Version::gameXres) {
          size_x = rect->right - rect->left;
          size_y = g_window_adjust_y + ((size_x - g_window_adjust_x) * Version::gameYres) / Version::gameXres;
        } else {
          size_y = rect->bottom - rect->top;
          size_x = g_window_adjust_x + ((size_y - g_window_adjust_y) * Version::gameXres) / Version::gameYres;
        }

        rect->left = rect->right - size_x;
        rect->bottom = rect->top + size_y;
      }
      break;
    case WMSZ_BOTTOMRIGHT:
      {
        int size_x, size_y;

        if (size_x_desired * Version::gameYres > size_y_desired * Version::gameXres) {
          size_x = rect->right - rect->left;
          size_y = g_window_adjust_y + ((size_x - g_window_adjust_x) * Version::gameYres) / Version::gameXres;
        } else {
          size_y = rect->bottom - rect->top;
          size_x = g_window_adjust_x + ((size_y - g_window_adjust_y) * Version::gameXres) / Version::gameYres;
        }

        rect->right = rect->left + size_x;
        rect->bottom = rect->top + size_y;
      }
      break;
    case WMSZ_LEFT:
    case WMSZ_RIGHT:
      {
        int size_y = g_window_adjust_y + (size_x_desired * Version::gameYres) / Version::gameXres;
        rect->top = (rect->top + rect->bottom) / 2 - size_y / 2;
        rect->bottom = rect->top + size_y;
      }
      break;
    case WMSZ_TOPLEFT:
      {
        int size_x, size_y;

        if (size_x_desired * Version::gameYres > size_y_desired * Version::gameXres) {
          size_x = rect->right - rect->left;
          size_y = g_window_adjust_y + ((size_x - g_window_adjust_x) * Version::gameYres) / Version::gameXres;
        } else {
          size_y = rect->bottom - rect->top;
          size_x = g_window_adjust_x + ((size_y - g_window_adjust_y) * Version::gameXres) / Version::gameYres;
        }

        rect->left = rect->right - size_x;
        rect->top = rect->bottom - size_y;
      }
      break;
    case WMSZ_TOPRIGHT:
      {
        int size_x, size_y;

        if (size_x_desired * Version::gameYres > size_y_desired * Version::gameXres) {
          size_x = rect->right - rect->left;
          size_y = g_window_adjust_y + ((size_x - g_window_adjust_x) * Version::gameYres) / Version::gameXres;
        } else {
          size_y = rect->bottom - rect->top;
          size_x = g_window_adjust_x + ((size_y - g_window_adjust_y) * Version::gameXres) / Version::gameYres;
        }

        rect->right = rect->left + size_x;
        rect->top = rect->bottom - size_y;
      }
      break;
    }
  }

  // Handles window messages that arrive by any means, message queue or by direct notification.
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
          // Commented out so that size changes are "invisible" to the code. TODO fix, once we have a proper scaling UI.
          //s_width = lParam1;
          //s_height = lParam2;
        }
        break;
      case WM_SIZING:
        {
          RECT *rect = reinterpret_cast<RECT *>(lParam);
          resize(int(wParam), rect);
          glViewport(0, 0, rect->right - rect->left - g_window_adjust_x, rect->bottom - rect->top - g_window_adjust_y); 
          break;
        }
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
    }

    if (s_core) {
      Frame::InputEvent iev;
      if (InputGatherWin32(&iev, window_handle, message, wParam, lParam))
      {
        s_core->Input(iev);
      }
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
        window_style |= WS_THICKFRAME;
    }

    DWORD window_style_ex = 0;
    
    int scrx = GetSystemMetrics(SM_CXSCREEN) - 30;
    int scry = GetSystemMetrics(SM_CYSCREEN) - 60;

    // Specify the window dimensions and get the border size
    RECT window_rectangle;
    window_rectangle.left = 0;
    window_rectangle.right = std::min(Version::gameXres, std::min(scrx, scry * Version::gameXres / Version::gameYres));
    window_rectangle.top = 0;
    window_rectangle.bottom = std::min(Version::gameYres, std::min(scry, scrx * Version::gameYres / Version::gameXres));
    if (!AdjustWindowRectEx(&window_rectangle, window_style, false, window_style_ex))
    {
      CHECK(false);
      return 0;
    }

    g_window_adjust_x = (window_rectangle.right - window_rectangle.left) - Version::gameXres;
    g_window_adjust_y = (window_rectangle.bottom - window_rectangle.top) - Version::gameYres;
    
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
        sSwapProc(FLAGS_development ? 0 : 1);
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

