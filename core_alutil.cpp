
#include "core_alutil.h"

#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#include <luabind/discard_result_policy.hpp>

#include "al/al.h"

#include "debug.h"

#include <vector>

using namespace std;


vector<ALuint> to_delete_buffers;
vector<ALuint> to_delete_sources;

class AlBufferID {
  int id;
  
public:
  AlBufferID() {
    alGenBuffers(1, &id);
  }
  ~AlBufferID() {
    to_delete_buffers.push_back(id);
  }
  
  int get() const {
    return id;
  }
};

class AlSourceID {
  int id;
  
public:
  AlSourceID() {
    alGenSources(1, &id);
  }
  ~AlSourceID() {
    to_delete_sources.push_back(id);
  }
  
  int get() const {
    return id;
  }
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
