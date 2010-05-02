
#include "core_glutil.h"

#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#include <luabind/discard_result_policy.hpp>

#include "GLee.h"

#include "debug.h"

#include <vector>

using namespace std;


#ifndef IPHONE
vector<int> to_delete_lists;
vector<int> to_delete_shaders;
vector<int> to_delete_programs;
vector<GLuint> to_delete_framebuffers;
vector<GLuint> to_delete_renderbuffers;
vector<GLuint> to_delete_textures;

class GlListID {
  int id;
  
public:
  GlListID() {
    id = glGenLists(1);
  }
  ~GlListID() {
    to_delete_lists.push_back(id);
  }
  
  int get() const {
    return id;
  }
};

class GlShader {
  int id;
  
public:
  GlShader(string typ) {
    int typi;
    if(typ == "VERTEX_SHADER") typi = GL_VERTEX_SHADER; else
    if(typ == "FRAGMENT_SHADER") typi = GL_FRAGMENT_SHADER; else
      CHECK(0);
    id = glCreateShader(typi);
  }
  ~GlShader() {
    to_delete_shaders.push_back(id);
  }
  
  int get() const {
    return id;
  }
};

class GlProgram {
  int id;
  
public:
  GlProgram() {
    id = glCreateProgram();
  }
  ~GlProgram() {
    to_delete_programs.push_back(id);
  }
  
  int get() const {
    return id;
  }
};

class GlFramebuffer {
  GLuint id;
  
public:
  GlFramebuffer() {
    glGenFramebuffers(1, &id);
  }
  ~GlFramebuffer() {
    to_delete_framebuffers.push_back(id);
  }
  
  int get() const {
    return id;
  }
};

class GlRenderbuffer {
  GLuint id;
  
public:
  GlRenderbuffer() {
    glGenRenderbuffers(1, &id);
  }
  ~GlRenderbuffer() {
    to_delete_renderbuffers.push_back(id);
  }
  
  int get() const {
    return id;
  }
};

class GlTexture {
  GLuint id;
  
public:
  GlTexture() {
    glGenTextures(1, &id);
    dprintf("glorp texid %d\n", id);
  }
  ~GlTexture() {
    to_delete_textures.push_back(id);
  }
  
  int get() const {
    return id;
  }
};
#endif

void glorp_glutil_init(lua_State *L) {
  {
    using namespace luabind;
    
    module(L)
    [
      #ifndef IPHONE
      class_<GlListID>("GlListID")
        .def(constructor<>())
        .def("get", &GlListID::get),
      class_<GlShader>("GlShader")
        .def(constructor<const std::string &>())
        .def("get", &GlShader::get),
      class_<GlProgram>("GlProgram")
        .def(constructor<>())
        .def("get", &GlProgram::get),
      class_<GlFramebuffer>("GlFramebuffer")
        .def(constructor<>())
        .def("get", &GlFramebuffer::get),
      class_<GlRenderbuffer>("GlRenderbuffer")
        .def(constructor<>())
        .def("get", &GlRenderbuffer::get),
      class_<GlTexture>("GlTexture")
        .def(constructor<>())
        .def("get", &GlTexture::get)
      #endif
    ];
  }
}
void glorp_glutil_tick() {
  #ifndef IPHONE      
  // siiiigh
  for(int i = 0; i < to_delete_lists.size(); i++) glDeleteLists(to_delete_lists[i], 1);
  for(int i = 0; i < to_delete_shaders.size(); i++) glDeleteShader(to_delete_shaders[i]);
  for(int i = 0; i < to_delete_programs.size(); i++) glDeleteProgram(to_delete_programs[i]);
  for(int i = 0; i < to_delete_framebuffers.size(); i++) glDeleteFramebuffers(1, &to_delete_framebuffers[i]);
  for(int i = 0; i < to_delete_renderbuffers.size(); i++) glDeleteRenderbuffers(1, &to_delete_renderbuffers[i]);
  for(int i = 0; i < to_delete_textures.size(); i++) glDeleteTextures(1, &to_delete_textures[i]);
  to_delete_lists.clear();
  to_delete_shaders.clear();
  to_delete_programs.clear();
  to_delete_framebuffers.clear();
  to_delete_renderbuffers.clear();
  to_delete_textures.clear();
  #endif
}
