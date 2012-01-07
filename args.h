#ifndef GLORP_ARGS
#define GLORP_ARGS

#include <map>
#include <string>

#include <boost/noncopyable.hpp>

using namespace std;

namespace Glorp {

  enum FlagSource { FS_DEFAULT, FS_FILE, FS_CLI };

  class ARGS_LinkageObject : boost::noncopyable {
  public:
    ARGS_LinkageObject(const string &id, string *writeto, FlagSource *source, const string &def, const string &descr);
    ARGS_LinkageObject(const string &id, int *writeto, FlagSource *source, int def, const string &descr);
    ARGS_LinkageObject(const string &id, bool *writeto, FlagSource *source, bool def, const string &descr);
    ARGS_LinkageObject(const string &id, float *writeto, FlagSource *source, float def, const string &descr);
  };

  map<string, string> getFlagDescriptions();

  void setInitFlagFile(const string &fname);
  void setInitFlagIgnore(int args);
}

#define DECLARE_VARIABLE(id, type) \
    extern type FLAGS_##id; \
    extern Glorp::FlagSource FLAGS_##id##_OVERRIDDEN;

#define DEFINE_VARIABLE(id, type, def, descr) \
  type FLAGS_##id;\
  Glorp::FlagSource FLAGS_##id##_OVERRIDDEN;\
  Glorp::ARGS_LinkageObject id##_linkage(#id, &FLAGS_##id, &FLAGS_##id##_OVERRIDDEN, def, descr); \
  char ***FLAGS_no##id __attribute__((unused)); // this only exists so we can't accidentally make two flags that only differ in "no"ness

#define DECLARE_string(id) DECLARE_VARIABLE(id, string)
#define DECLARE_int(id) DECLARE_VARIABLE(id, int)
#define DECLARE_bool(id) DECLARE_VARIABLE(id, bool)
#define DECLARE_float(id) DECLARE_VARIABLE(id, float)

#define DEFINE_string(id, def, descr) DEFINE_VARIABLE(id, string, def, descr)
#define DEFINE_int(id, def, descr) DEFINE_VARIABLE(id, int, def, descr)
#define DEFINE_bool(id, def, descr) DEFINE_VARIABLE(id, bool, def, descr)
#define DEFINE_float(id, def, descr) DEFINE_VARIABLE(id, float, def, descr)

#endif
