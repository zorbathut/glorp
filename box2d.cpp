#ifdef GLORP_BOX2D

#include <Box2D/Box2D.h>

#include "debug.h"
#include "util.h"

#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#include <luabind/adopt_policy.hpp>

#include <boost/static_assert.hpp>

map<b2Body *, b2World *> bodyassociations;
class BodyWrapper : public auto_ptr_customized<b2Body, BodyWrapper> {
public:
  BodyWrapper(b2Body *item) : auto_ptr_customized<b2Body, BodyWrapper>(item) { };
  BodyWrapper(BodyWrapper &item) : auto_ptr_customized<b2Body, BodyWrapper>(item) { };
  BodyWrapper(const auto_ptr_customized_ref<b2Body, BodyWrapper> &item) : auto_ptr_customized<b2Body, BodyWrapper>(item) { };
  static void cleanup(b2Body *item) {
    bodyassociations[item]->DestroyBody(item);  // huuuuurj
    bodyassociations.erase(item);
  }
};
b2Body *CreateBody(b2World *world, b2BodyDef *def) {
  b2Body *n = world->CreateBody(def);
  bodyassociations[n] = world;
  return n;
}




void glorp_box2d_init(lua_State *L) {
  using namespace luabind;
  
  module(L, "b2")
  [
    class_<b2Vec2>("Vec2")
      .def(constructor<float, float>())
      .def(constructor<>())
      .def_readwrite("x", &b2Vec2::x)
      .def_readwrite("y", &b2Vec2::y)
      .def("Length", &b2Vec2::Length)
      .def("LengthSquared", &b2Vec2::LengthSquared)
      .def("Normalize", &b2Vec2::Normalize),
    class_<b2Body, BodyWrapper>("Body"),
    class_<b2World>("World")
      .def(constructor<b2Vec2, bool>())
      .def("CreateBody", &CreateBody, adopt_container<BodyWrapper>(result) | dependency(result, _1))
  ];
};

#endif
