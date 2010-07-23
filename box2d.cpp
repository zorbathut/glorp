#ifdef GLORP_BOX2D

#include <Box2D/Box2D.h>

#include "debug.h"
#include "util.h"

#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#include <luabind/adopt_policy.hpp>

#include <boost/static_assert.hpp>
#include <boost/noncopyable.hpp>

#include <set>

using namespace std;

/* okay let's talk this over

We can mostly preserve b2World inside a wrapper, though we need for b2World's creation functions to do weirder, stranger things.

Most specifically, we need to dynamically allocate b2BodyWrapper instances, and we need to keep track of which b2BodyWrapper instances refer to which b2Body.

Then, if we need to delete a b2Body for any reason, we can wipe the b2BodyWrapper instances first.

Additionally, this means that none of the critical structures are actually owned by luajit besides the b2World. We clean them up manually.

*/

/******************************
                    UTILITIES
*******************************/

class Vec2Array : boost::noncopyable{
public:
  vector<b2Vec2> elements;

  Vec2Array(int len) : elements(len) { };

  b2Vec2 &element(int id) {
    return elements[id - 1];
  }
};

/******************************
                    BODY WRAPPER
*******************************/

class BodyWrapper : boost::noncopyable {
public:
  b2Body *body;

  const b2Vec2 &GetPosition() const {
    CHECK(body);
    return body->GetPosition();
  }
  float GetAngle() const {
    CHECK(body);
    return body->GetAngle();
  }
  
  b2Vec2 GetLinearVelocity() const {
    CHECK(body);
    return body->GetLinearVelocity();
  }
  void SetLinearVelocity(const b2Vec2 &lv) {
    CHECK(body);
    body->SetLinearVelocity(lv);
  }
  
  float GetAngularVelocity() const {
    CHECK(body);
    return body->GetAngularVelocity();
  }
  void SetAngularVelocity(float lv) {
    CHECK(body);
    body->SetAngularVelocity(lv);
  }
  
  ~BodyWrapper();
};
map<b2Body *, set<BodyWrapper *> > bwrapper_owned;

BodyWrapper *CreateBody(b2World *world, b2BodyDef *def) {
  b2Body *newbody = world->CreateBody(def);
  CHECK(!bwrapper_owned.count(newbody));
  BodyWrapper *bwrap = new BodyWrapper();
  bwrap->body = newbody;
  bwrapper_owned[newbody].insert(bwrap);
  return bwrap;
}

BodyWrapper::~BodyWrapper() {
  if(body) {
    CHECK(bwrapper_owned[body].count(this));
    bwrapper_owned[body].erase(this);
    if(!bwrapper_owned[body].size())
      bwrapper_owned.erase(body);
  }
}

/******************************
                    FIXTURE WRAPPER
*******************************/

class FixtureWrapper : boost::noncopyable {
public:
  b2Fixture *fixture;

  ~FixtureWrapper();
};
map<b2Fixture *, set<FixtureWrapper *> > fwrapper_owned;


FixtureWrapper *WrapFixture(b2Fixture *newfixt) {
  CHECK(!fwrapper_owned.count(newfixt), "%d", fwrapper_owned.find(newfixt)->second.size());
  FixtureWrapper *fwrap = new FixtureWrapper();
  fwrap->fixture = newfixt;
  fwrapper_owned[newfixt].insert(fwrap);
  return fwrap;
}
FixtureWrapper *CreateFixtureFromDef(BodyWrapper *body, b2FixtureDef *def) {
  CHECK(body->body);
  return WrapFixture(body->body->CreateFixture(def));
}
FixtureWrapper *CreateFixtureFromShape(BodyWrapper *body, b2Shape *def) {
  CHECK(body->body);
  return WrapFixture(body->body->CreateFixture(def, 0));
}
FixtureWrapper *CreateFixtureFromShapeDensity(BodyWrapper *body, b2Shape *def, float density) {
  CHECK(body->body);
  return WrapFixture(body->body->CreateFixture(def, density));
}
FixtureWrapper::~FixtureWrapper() {
  if(fixture) {
    CHECK(fwrapper_owned[fixture].count(this));
    fwrapper_owned[fixture].erase(this);
    if(!fwrapper_owned[fixture].size()) {
      fwrapper_owned.erase(fixture);
    }
  }
}

/******************************
            DESTROYMENT FUNCTIONS
*******************************/

void DestroyFixture(BodyWrapper *bwrap, FixtureWrapper *fwrap) {
  CHECK(bwrap->body);
  CHECK(fwrap->fixture);
  
  // first, clear all instances of that fixture
  b2Fixture *fixt = fwrap->fixture;
  const set<FixtureWrapper*> &st = fwrapper_owned[fixt];
  for(set<FixtureWrapper*>::iterator itr = st.begin(); itr != st.end(); itr++) {
    (*itr)->fixture = NULL;
  }
  fwrapper_owned.erase(fixt);
  
  bwrap->body->DestroyFixture(fixt);
}

void DestroyBody(b2World *world, BodyWrapper *bwrap) {
  CHECK(bwrap->body);
  
  b2Body *body = bwrap->body;
  
  // we need to wipe all the fixtures associated with the body, first
  b2Fixture *cfxt = body->GetFixtureList();
  while(cfxt) {
    const set<FixtureWrapper*> &st = fwrapper_owned[cfxt];
    for(set<FixtureWrapper*>::iterator itr = st.begin(); itr != st.end(); itr++) {
      (*itr)->fixture = NULL;
    }
    fwrapper_owned.erase(cfxt);
    
    cfxt = cfxt->GetNext();
  }
  
  // wipe all the instances of this body
  const set<BodyWrapper*> &st = bwrapper_owned[body];
  for(set<BodyWrapper*>::iterator itr = st.begin(); itr != st.end(); itr++) {
    (*itr)->body = NULL;
  }
  bwrapper_owned.erase(body);
  
  world->DestroyBody(body);
}



void PolygonShapeSetFromVec2Array(b2PolygonShape *b2ps, Vec2Array *v2a) {
  b2ps->Set(&v2a->elements[0], v2a->elements.size());
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
std::ostream &operator<<(std::ostream &ostr, const BodyWrapper &vec) {
  ostr << "[b2BodyWrapper " << vec.body << "]"; return ostr; }
std::ostream &operator<<(std::ostream &ostr, const b2World &vec) {
  ostr << "[b2World " << &vec << "]"; return ostr; }
std::ostream &operator<<(std::ostream &ostr, const b2BodyDef &vec) {
  ostr << "[b2BodyDef " << &vec << "]"; return ostr; }
std::ostream &operator<<(std::ostream &ostr, const b2Shape &vec) {
  ostr << "[b2Shape " << &vec << "]"; return ostr; }
std::ostream &operator<<(std::ostream &ostr, const b2PolygonShape &vec) {
  ostr << "[b2PolygonShape " << &vec << "]"; return ostr; }
std::ostream &operator<<(std::ostream &ostr, const b2CircleShape &vec) {
  ostr << "[b2CircleShape " << &vec << "]"; return ostr; }
std::ostream &operator<<(std::ostream &ostr, const b2Fixture &vec) {
  ostr << "[b2Fixture " << &vec << "]"; return ostr; }
std::ostream &operator<<(std::ostream &ostr, const FixtureWrapper &vec) {
  ostr << "[b2FixtureWrapper " << vec.fixture << "]"; return ostr; }
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
    class_<Vec2Array>("Vec2Array")
      .def(constructor<int>())
      .def("element", &Vec2Array::element),
    class_<BodyWrapper>("Body")
      .def("CreateFixture", &CreateFixtureFromDef, adopt(result))
      .def("CreateFixture", &CreateFixtureFromShape, adopt(result))
      .def("CreateFixture", &CreateFixtureFromShapeDensity, adopt(result))
      .def("DestroyFixture", &DestroyFixture)
      .property("position", &BodyWrapper::GetPosition)
      .property("angle", &BodyWrapper::GetAngle)
      .property("linearvelocity", &BodyWrapper::GetLinearVelocity, &BodyWrapper::SetLinearVelocity)
      .property("angularvelocity", &BodyWrapper::GetAngularVelocity, &BodyWrapper::SetAngularVelocity)
      .def(tostring(self)),
    class_<b2World>("World")
      .def(constructor<b2Vec2, bool>())
      .def("CreateBody", &CreateBody, adopt(result))
      .def("DestroyBody", &DestroyBody, adopt(result))
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
      .def("Set", &PolygonShapeSetFromVec2Array)
      .def(tostring(self)),
    class_<b2CircleShape, b2Shape>("CircleShape")
      .def(constructor<>())
      .def_readwrite("m_p", &b2CircleShape::m_p)
      .def_readwrite("m_radius", &b2CircleShape::m_radius)
      .def(tostring(self)),
    class_<FixtureWrapper>("Fixture")
      .def(tostring(self)),
    class_<b2FixtureDef>("FixtureDef")
      .def(constructor<>())
      .property("shape", &FixtureDefGetShape, &FixtureDefSetShape)
      .def_readwrite("density", &b2FixtureDef::density)
      .def_readwrite("friction", &b2FixtureDef::friction)
      .def(tostring(self))
  ];
  
  lua_getglobal(L, "b2");
  lua_pushnumber(L, b2_maxPolygonVertices);
  lua_setfield(L, -2, "maxPolygonVertices");
  lua_pop(L, 1);
};

#endif
