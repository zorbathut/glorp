 
  {
    #ifdef WIN32
      SendMessage(get_first_handle(), WM_SETICON, ICON_BIG, (LPARAM)LoadImage(GetModuleHandle(0), MAKEINTRESOURCE(101), IMAGE_ICON, 32, 32, 0));
      SendMessage(get_first_handle(), WM_SETICON, ICON_SMALL, (LPARAM)LoadImage(GetModuleHandle(0), MAKEINTRESOURCE(101), IMAGE_ICON, 16, 16, 0));
    #endif
    
    #ifdef LINUX
      glorp_set_icons();
    #endif
    
    
  dprintf("EXTENSIONS: %s", glGetString(GL_EXTENSIONS));
  dprintf("VENDOR: %s", glGetString(GL_VENDOR));
  dprintf("RENDERER: %s", glGetString(GL_RENDERER));
  dprintf("VERSION: %s", glGetString(GL_VERSION));
    
  if(atof((const char*)glGetString(GL_VERSION)) < 2.0 && !FLAGS_permit_ogl1) {
    CHECK_MESSAGE(false, "%s currently requires OpenGL 2.0, which your computer doesn't seem to have.\n\nUpdating your video drivers might fix the problem, or it might not. Sorry!\n\nI've created a datafile including some information that may help Mandible Games fix\nthe error in future versions. It contains no personally identifying information.\n\nMay I send this to Mandible?");
    return;
  }
  
  }