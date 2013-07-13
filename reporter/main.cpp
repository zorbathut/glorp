
#include <curl/curl.h>

#include <zlib.h>

#include <string>
#include <map>
#include <cassert>
#include <cstdio>

#include "version.h"

using namespace std;

#if defined(WIN32)

#include <windows.h> // :D

int Message(const string &text, bool yesno) {
  int rv = MessageBox(NULL, text.c_str(), "Croosh Report", (yesno ? MB_OKCANCEL : MB_OK) | MB_SETFOREGROUND);
  if(rv == IDOK)
    return true;
  return false;
}

#elif defined(MACOSX)

// implemented in os_ui_osx.mm

#else

#include <FL/fl_ask.H>

int Message(const string &text, bool yesno) {
  if(!yesno) {
    fl_message("%s", text.c_str());
    return false;
  } else {
    return fl_ask("%s", text.c_str());
  }
}

#endif

using namespace std;

size_t writefunc(void *ptr, size_t size, size_t nmemb, void *lulz) {
  printf("appendinating %d\n", size * nmemb);
  string *odat = (string*)lulz;
  *odat += string((char*)ptr, (char*)ptr + size * nmemb);
  return size * nmemb;
}

string cee(CURL *curl, const string &str) {
  char *escy = curl_easy_escape(curl, str.c_str(), str.size());
  assert(escy);
  assert(strlen(escy) >= str.size());
  string rv = escy;
  curl_free(escy);
  return rv;
}

string request(string url, const map<string, string> &posts) {
  printf("Entering request\n");
  CURL *handle = curl_easy_init();
  assert(handle);
  
  char errbuf[CURL_ERROR_SIZE];
  string out;
  
  curl_easy_setopt(handle, CURLOPT_ERRORBUFFER, errbuf);
  curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, writefunc);
  curl_easy_setopt(handle, CURLOPT_WRITEDATA, &out);
  
  string urlencode;
  for(map<string, string>::const_iterator itr = posts.begin(); itr != posts.end(); itr++) {
    if(urlencode.size())
      urlencode += "&";
    urlencode += cee(handle, itr->first) + "=" + cee(handle, itr->second);
  }
  printf("urlencode is %s\n", urlencode.c_str());
  
  curl_easy_setopt(handle, CURLOPT_POSTFIELDS, urlencode.c_str());
  curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
  printf("Sending . . .\n");
  if(curl_easy_perform(handle)) {
    printf("Error: %s\n", errbuf);
    assert(0);
  }
  printf("Done!\n");
  
  curl_easy_cleanup(handle);
  
  return out;
}

int main(int argc, const char *argv[]) {
  printf("%d", argc);
  assert(argc == 8);
  
  printf("REPORTER IS YOUR FREND\n");
  
  const char *msg = argv[7];
  if(strlen(msg) == 0)
  {
    msg = "Something unexpected has happened, and %s has been shut down.\n\nI've created a datafile including some information that may help Mandible Games\nfix the error in future versions. It contains no personally identifying information.\n\nMay I send this to Mandible?";
  }
  
  char strbuff[1024];
  sprintf(strbuff, msg, Glorp::Version::gameFullname);
  
  if(!Message(strbuff, true)) {
    unlink(argv[2]);
    return 1;
  }
  
  assert(curl_global_init(CURL_GLOBAL_ALL) == 0);
  
  string url = "http://crashlog.mandible-games.com:911/weekly.php";
  
  map<string, string> parms;
  string res;
  parms["instruction"] = "request";
  parms["version"] = argv[3];
  parms["file"] = argv[4];
  parms["line"] = argv[5];
  parms["exesize"] = argv[6];
  
  res = request(url, parms);
  
  if(res != "OK") {
    Message(res, false);
    return 1;
  }
  
  string krlog;
  {
    z_stream comp;
    comp.zalloc = Z_NULL;
    comp.zfree = Z_NULL;
    comp.opaque = 0;
    assert(deflateInit2(&comp, 5, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY) == Z_OK);
    
    FILE *fil = fopen(argv[2], "rb");
    assert(fil);
    char buff[65536];
    char out[65536];
    comp.next_out = (Byte*)out;
    comp.avail_out = sizeof(out);
    int dat;
    while((dat = fread(buff, 1, sizeof(buff), fil)) > 0) {
      comp.next_in = (Byte*)buff;
      comp.avail_in = dat;
      while(comp.avail_in) {
        assert(deflate(&comp, 0) == Z_OK);
        krlog += string(out, out + sizeof(out) - comp.avail_out);
        assert(krlog.size() == comp.total_out);
        comp.next_out = (Byte*)out;
        comp.avail_out = sizeof(out);
      }
    }
    
    while(1) {
      int rv = deflate(&comp, Z_FINISH);
      assert(rv == Z_OK || rv == Z_STREAM_END);
      krlog += string(out, out + sizeof(out) - comp.avail_out);
      assert(krlog.size() == comp.total_out);
      comp.next_out = (Byte*)out;
      comp.avail_out = sizeof(out);
      if(rv == Z_STREAM_END)
        break;
    }
    
    fclose(fil);
    unlink(argv[2]);

    printf("compressed %d to %d\n", (int)comp.total_in, (int)comp.total_out);
    assert(krlog.size() == comp.total_out);
    
    assert(deflateEnd(&comp) == Z_OK);
  }
  
  parms["dump"] = krlog;
  parms["instruction"] = "dump";

  res = request(url, parms);
  
  printf("res is %s\n", res.c_str());
  
  if(res != "OK") {
    Message(res, false);
    return 1;
  } else {
    Message("Dump sent, thank you for reporting the error!", false);
    return 0;
  }
};
