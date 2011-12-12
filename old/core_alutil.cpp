
#include "core_alutil.h"

#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#include <luabind/discard_result_policy.hpp>
#include <luabind/adopt_policy.hpp>

#include "AL/al.h"

#include "debug.h"
#include "sound.h"

#include <vector>

#include <boost/noncopyable.hpp>

using namespace std;


static vector<ALuint> to_delete_buffers;
static vector<ALuint> to_delete_sources;

class AlBufferID : boost::noncopyable {
  ALuint id;
  
public:
  AlBufferID() {
    alGenBuffers(1, &id);
  }
  AlBufferID(int _id) {
    id = _id;
  }
  ~AlBufferID() {
    to_delete_buffers.push_back(id);
  }
  
  ALuint get() const {
    return id;
  }
};

class AlSourceID : boost::noncopyable {
  ALuint id;
  
public:
  AlSourceID() {
    alGenSources(1, &id);
  }
  ~AlSourceID() {
    to_delete_sources.push_back(id);
  }
  
  ALuint get() const {
    return id;
  }
};

AlBufferID *LoadSoundWrapped(const char *inp) {
  int snd = loadSound(inp);
  CHECK(snd);
  return new AlBufferID(snd);
};

void glorp_alutil_init(lua_State *L) {
  {
    using namespace luabind;
    
    module(L)
    [
      class_<AlBufferID>("AlBufferID")
        .def(constructor<>())
        .def("get", &AlBufferID::get),
      class_<AlSourceID>("AlSourceID")
        .def(constructor<>())
        .def("get", &AlSourceID::get),
      def("LoadSound", &LoadSoundWrapped, adopt(result))
    ];
  }
}
void glorp_alutil_tick() {
  CHECK(alGetError() == AL_NO_ERROR);
  for(int i = 0; i < to_delete_sources.size(); i++) alDeleteSources(1, &to_delete_sources[i]);
  for(int i = 0; i < to_delete_buffers.size(); i++) alDeleteBuffers(1, &to_delete_buffers[i]);
  CHECK(alGetError() == AL_NO_ERROR);
  to_delete_sources.clear();
  to_delete_buffers.clear();
}
