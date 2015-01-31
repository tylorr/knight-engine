#include "uniform_manager.h"
#include "uniform.h"

#include <logog.hpp>
#include <hash.h>
#include <temp_allocator.h>

using namespace foundation;

namespace knight {

ShaderProgram::~ShaderProgram() {
  if (vertex_handle_) {
    glDeleteShader(vertex_handle_);
  }

  if (fragment_handle_) {
    glDeleteShader(fragment_handle_);
  }
  
  if (handle_) {
    glDeleteProgram(handle_);
  }
}

void ShaderProgram::Initialize(UniformManager &uniform_manager, const char *source) {
  auto getDefine = [](GLenum type) {
    switch (type) {
      case GL_VERTEX_SHADER: return "#define VERTEX\n";
      case GL_FRAGMENT_SHADER: return "#define FRAGMENT\n";
      default: return "";
    }
  };

  auto createAndAttachShader = [this, &getDefine, &source](GLenum type) {
    auto shader_handle = glCreateShader(type);

    const char *full_source[3] = { "#version 440\n", getDefine(type), source };

    glShaderSource(shader_handle, 3, full_source, nullptr);
    glCompileShader(shader_handle);

    auto result = GLint{};
    glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
      auto length = GLint{};
      glGetShaderiv(shader_handle, GL_INFO_LOG_LENGTH, &length);

      if (length) {
        char info_log[length];
        glGetShaderInfoLog(shader_handle, length, nullptr, info_log);
        XASSERT(result != GL_FALSE, "Shader compile error:\n%s", info_log);
      }
    }

    glAttachShader(handle_, shader_handle);
    return shader_handle;
  };

  handle_ = glCreateProgram();

  vertex_handle_ = createAndAttachShader(GL_VERTEX_SHADER);
  fragment_handle_ = createAndAttachShader(GL_FRAGMENT_SHADER);
  
  auto result = GLint{};
  glLinkProgram(handle_);
  glGetProgramiv(handle_, GL_LINK_STATUS, &result);
  if (result == GL_FALSE) {
    auto length = GLint{};
    glGetProgramiv(handle_, GL_INFO_LOG_LENGTH, &length);

    if (length) {
      char info_log[length];
      glGetProgramInfoLog(handle_, length, nullptr, info_log);
      XASSERT(result != GL_FALSE, "Shader program link error:\n%s", info_log);
    }
  }

  auto max_uniform_name_length = GLint{};
  glGetProgramiv(handle_, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_uniform_name_length);

  auto uniform_count = GLint{};
  glGetProgramiv(handle_, GL_ACTIVE_UNIFORMS, &uniform_count);

  for (auto i = 0; i < uniform_count; ++i) {
    char name[max_uniform_name_length];

    auto value_size = GLint{};
    auto value_type = GLenum{};

    glGetActiveUniform(handle_, i, max_uniform_name_length, nullptr, 
                       &value_size, &value_type, name);

    auto location = GetUniformLocation(name);
    uniform_manager.Create(*this, location, name, value_type);
  }
}

void ShaderProgram::Bind() const {
  XASSERT(handle_, "Trying to bind an uninitialized shader program");
  glUseProgram(handle_);
}

void ShaderProgram::Unbind() const {
  XASSERT(handle_, "Trying to unbind an uninitialized shader program");
  
  auto current_program = GLint{};
  glGetIntegerv(GL_CURRENT_PROGRAM, &current_program);

  if (handle_ == current_program) {
    glUseProgram(0);
  }
}

GLint ShaderProgram::GetUniformLocation(const GLchar *name) const {
  return glGetUniformLocation(handle_, name);
}

GLint ShaderProgram::GetAttributeLocation(const GLchar *name) const {
  return glGetAttribLocation(handle_, name);
}

} // namespace knight
