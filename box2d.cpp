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
  CHECK(!bodyassociations.count(n));
  bodyassociations[n] = world;
  return n;
}

map<b2Fixture *, b2Body *> fixtureassociations;
class FixtureWrapper : public auto_ptr_customized<b2Fixture, FixtureWrapper> {
public:
  FixtureWrapper(b2Fixture *item) : auto_ptr_customized<b2Fixture, FixtureWrapper>(item) { };
  FixtureWrapper(FixtureWrapper &item) : auto_ptr_customized<b2Fixture, FixtureWrapper>(item) { };
  FixtureWrapper(const auto_ptr_customized_ref<b2Fixture, FixtureWrapper> &item) : auto_ptr_customized<b2Fixture, FixtureWrapper>(item) { };
  static void cleanup(b2Fixture *item) {
    fixtureassociations[item]->DestroyFixture(item);  // huuuuurj
    fixtureassociations.erase(item);
  }
};
b2Fixture *CreateFixtureFromDef(b2Body *body, b2FixtureDef *def) {
  b2Fixture *n = body->CreateFixture(def);
  CHECK(!fixtureassociations.count(n));
  fixtureassociations[n] = body;
  return n;
}
b2Fixture *CreateFixtureFromShape(b2Body *body, b2Shape *def) {
  b2Fixture *n = body->CreateFixture(def, 0);
  CHECK(!fixtureassociations.count(n));
  fixtureassociations[n] = body;
  return n;
}
b2Fixture *CreateFixtureFromShapeDensity(b2Body *body, b2Shape *def, float density) {
  b2Fixture *n = body->CreateFixture(def, density);
  CHECK(!fixtureassociations.count(n));
  fixtureassociations[n] = body;
  return n;
}


const b2Shape *FixtureDefGetShape(const b2FixtureDef *a) {
  return a->shape;
}
void FixtureDefSetShape(b2FixtureDef *a, b2Shape *shape) {
  a->shape = shape;
}

std::ostream &operator<<(std::ostream &ostr, const b2Vec2 &vec) {
  ostr << "(" << vec.x << ", " << vec.y << ")"; return ostr; }
std::ostream &operator<<(std::ostream &ostr, const b2Body &vec) {
  ostr << "[b2Body " << &vec << "]"; return ostr; }
std::ostream &operator<<(std::ostream &ostr, const b2World &vec) {
  ostr << "[b2World " << &vec << "]"; return ostr; }
std::ostream &operator<<(std::ostream &ostr, const b2BodyDef &vec) {
  ostr << "[b2BodyDef " << &vec << "]"; return ostr; }
std::ostream &operator<<(std::ostream &ostr, const b2Shape &vec) {
  ostr << "[b2Shape " << &vec << "]"; return ostr; }
std::ostream &operator<<(std::ostream &ostr, const b2PolygonShape &vec) {
  ostr << "[b2PolygonShape " << &vec << "]"; return ostr; }
std::ostream &operator<<(std::ostream &ostr, const b2Fixture &vec) {
  ostr << "[b2Fixture " << &vec << "]"; return ostr; }
std::ostream &operator<<(std::ostream &ostr, const b2FixtureDef &vec) {
  ostr << "[b2FixtureDef " << &vec << "]"; return ostr; }

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
      .def("Normalize", &b2Vec2::Normalize)
      .def(tostring(self)),
    class_<b2Body>("Body")
      .def("CreateFixture", &CreateFixtureFromDef, adopt_container<FixtureWrapper>(result) | dependency(result, _1) | dependency(_1, result))
      .def("CreateFixture", &CreateFixtureFromShape, adopt_container<FixtureWrapper>(result) | dependency(result, _1) | dependency(_1, result))
      .def("CreateFixture", &CreateFixtureFromShapeDensity, adopt_container<FixtureWrapper>(result) | dependency(result, _1) | dependency(_1, result))
      .property("position", &b2Body::GetPosition)
      .property("angle", &b2Body::GetAngle)
      .def(tostring(self)),
    class_<b2World>("World")
      .def(constructor<b2Vec2, bool>())
      .def("CreateBody", &CreateBody, adopt_container<BodyWrapper>(result) | dependency(result, _1) | dependency(_1, result))
      .def("Step", &b2World::Step)
      .def("ClearForces", &b2World::ClearForces)
      .def(tostring(self)),
    class_<b2BodyDef>("BodyDef")
      .def(constructor<>())
      .def_readwrite("position", &b2BodyDef::position)
      .def_readwrite("type", &b2BodyDef::type).enum_("constants") [ value("staticBody", b2_staticBody), value("kinematicBody", b2_kinematicBody), value("dynamicBody", b2_dynamicBody) ]
      .def(tostring(self)),
    class_<b2Shape>("Shape")
      .def(tostring(self)),
    class_<b2PolygonShape, b2Shape>("PolygonShape")
      .def(constructor<>())
      .def("SetAsBox", (void (b2PolygonShape::*)(float, float))&b2PolygonShape::SetAsBox)
      .def("SetAsBox", (void (b2PolygonShape::*)(float, float, const b2Vec2 &, float))&b2PolygonShape::SetAsBox)
      .def(tostring(self)),
    class_<b2Fixture>("Fixture")
      .def(tostring(self)),
    class_<b2FixtureDef>("FixtureDef")
      .def(constructor<>())
      .property("shape", &FixtureDefGetShape, &FixtureDefSetShape)
      .def_readwrite("density", &b2FixtureDef::density)
      .def_readwrite("friction", &b2FixtureDef::friction)
      .def(tostring(self))
  ];
};

#endif
