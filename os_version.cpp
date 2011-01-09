
#include "os.h"

#include "debug.h"
#include "parse.h"
#include "version.h"

#include <unistd.h>
#include <sys/stat.h>

#ifdef WIN32

#include <windows.h>
#include <shlobj.h>
#include <psapi.h>

static const string directory_delimiter = "\\";

string getConfigDirectory() {
  char buff[MAX_PATH + 1];
  SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, buff);
  //dprintf("Home directory: %s\n", buff);
  return string(buff) + "\\" + game_fullname + "\\";
}

void wrap_mkdir(const string &str) {
  mkdir(str.c_str());
}

string getTempFilename() {
  char buff[MAX_PATH + 1];
  GetTempPath(sizeof(buff), buff);
  char fname[MAX_PATH + 1];
  GetTempFileName(buff, game_slug, 0, fname);
  return fname;
}

#else

#include <cstring>
#include <stdlib.h>

static const string directory_delimiter = "/";

string getConfigDirectory() {
  string bf = getenv("HOME");
  //dprintf("Home directory: %s\n", bf.c_str());
  return bf + "/." + game_fullname + "/";
}

void wrap_mkdir(const string &str) {
  mkdir(str.c_str(), 0700);
}

string getTempFilename() {
  char temparg[128] = "/tmp/";
  strcat(temparg, game_slug);
  strcat(temparg, "-XXXXXX");
  close(mkstemp(temparg));
  return temparg;
}

#endif

void makeConfigDirectory() {
  CHECK(directory_delimiter.size() == 1);
  vector<string> tok = tokenize(getConfigDirectory(), directory_delimiter);
  
  string cc;
  if(getConfigDirectory().size() && getConfigDirectory()[0] == directory_delimiter[0])
    cc = directory_delimiter;
  
  for(int i = 0; i < tok.size(); i++) {
    if(cc.size())
      cc += directory_delimiter;
    cc += tok[i];
    wrap_mkdir(cc);
  }
}
