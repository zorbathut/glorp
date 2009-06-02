#ifndef DNET_DEBUG
#define DNET_DEBUG






#include <deque>
#include <string>

#ifndef SUPPRESS_GLOP

  #include <Glop/Base.h>

  #define dprintf LOGF

#else

#define CHECK(expression) ((expression) ? (void)(0) : (dprintf("Error at %s:%d - %s\n", __FILE__, __LINE__, #expression), exit(1)))
int dprintf(const char *bort, ...);

#endif

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
