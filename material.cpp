#include "material.h"

#include "shader_cache.h"

#include <GL/glew.h>
#include <GL/glfw.h>

using std::string;

Material::Material(const string &shader_key) {
  shader_program_ = ShaderCache::GetShaderProgram(shader_key);
}
