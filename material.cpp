#include "material.h"

#include "shader_cache.h"

#include <glm/gtc/type_ptr.hpp>

#include <GL/glew.h>
#include <GL/glfw.h>

using std::string;
using glm::mat4;
using glm::vec4;

Material::Material(const GLuint &shader_program) :
  shader_program_(shader_program),
  transparent_(false) { }

bool Material::HasProperty(const string &name) {
  return GetLocation(name) >= 0;
}

const GLint &Material::GetLocation(const string &name) {
  UniformMap::const_iterator it = uniform_map_.find(name);
  if (it != uniform_map_.end()) {
    return it->second;
  } else {
    return uniform_map_[name] = glGetUniformLocation(shader_program_, name.c_str());
  }
}

void Material::SetMatrix(const string &name, const mat4 &matrix) {
  glUniformMatrix4fv(GetLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
}

void Material::SetVector(const string &name, const vec4 &vector) {
  glUniform4fv(GetLocation(name), 1, glm::value_ptr(vector));
}

void Material::SetFloat(const string &name, const float &value) {
  glUniform1f(GetLocation(name), value);
}

