#ifndef DNET_DEBUG
#define DNET_DEBUG






#include <deque>
#include <string>

#include <Glop/Base.h>

#define dprintf LOGF

using namespace std;

/*************
 * CHECK/TEST macros
 */
 
class StackPrinter {
public:
  virtual void Print() const = 0;

  StackPrinter();
  virtual ~StackPrinter();
};

class StackString : public StackPrinter {
public:
  void Print() const;

  StackString(const string &str);

private:
  string str_;
};

deque<string> &dbgrecord();

void registerCrashFunction(void (*)());
void unregisterCrashFunction(void (*)());
 
void disableStackTrace();

extern void *stackStart;




#define printf FAILURE
 
#endif
