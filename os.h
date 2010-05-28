#ifndef DNET_OS
#define DNET_OS

#include <string>
#include <vector>

using namespace std;

// Debug stuff
void outputDebugString(const string &str);

void dumpStackTrace();

bool isUnoptimized();

// OS stuff
void seriouslyCrash() __attribute__((__noreturn__)); // apparently this is needed. Why? Because Cygwin is stupid.

string getDesktopDirectory();
string getConfigDirectory(); // oy
void makeConfigDirectory();

string getTempFilename();

void SpawnProcess(const string &program, const vector<string> &params);

int exesize();

int memory_usage(); // someday this will break when I have a 64-bit program.

#endif
