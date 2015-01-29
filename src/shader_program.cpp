#include "shader_program.h"
#include "uniform_manager.h"
#include "uniform.h"
#include "knight_string.h"

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
  handle_ = glCreateProgram();

  auto createAndAttachShader = [this, &source](GLenum type) {
    auto shader_handle = glCreateShader(type);

    TempAllocator4096 temp_allocator;
    auto full_source = Array<char>{temp_allocator};
    string::append(full_source, "#version 440\n");
    switch(type) {
      case GL_VERTEX_SHADER:
        string::append(full_source, "#define VERTEX\n");
        break;
      case GL_FRAGMENT_SHADER:
        string::append(full_source, "#define FRAGMENT\n");
        break;
    }
    string::append(full_source, source);

    auto c_full_source = string::c_str(full_source);
    glShaderSource(shader_handle, 1, &c_full_source, nullptr);
    glCompileShader(shader_handle);

    auto result = GLint{};
    glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &result);
    XASSERT(result != GL_FALSE, 
            "Failed to compile shader %d:\n%s", 
            shader_handle, string::c_str(shader_program::GetShaderInfoLog(handle_, shader_handle)));

    glAttachShader(handle_, shader_handle);
    return shader_handle;
  };

  vertex_handle_ = createAndAttachShader(GL_VERTEX_SHADER);
  fragment_handle_ = createAndAttachShader(GL_FRAGMENT_SHADER);
  
  GLint result;
  glLinkProgram(handle_);
  glGetProgramiv(handle_, GL_LINK_STATUS, &result);
  XASSERT(result != GL_FALSE, "Failed to link program: %s", string::c_str(shader_program::GetProgramInfoLog(handle_)));

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

namespace shader_program {

  Array<char> GetProgramInfoLog(GLuint program_handle) {
    GLint length;
    glGetProgramiv(program_handle, GL_INFO_LOG_LENGTH, &length);

    auto info_string= Array<char>{memory_globals::default_scratch_allocator()};
    if (length) {
      char info_log[length];
      glGetProgramInfoLog(program_handle, length, nullptr, info_log);
      string::append(info_string, info_log);
    }
    return info_string;
  }

  Array<char> GetShaderInfoLog(
      GLuint program_handle, 
      GLuint shader_handle) {
    GLint length;
    glGetShaderiv(program_handle, GL_INFO_LOG_LENGTH, &length);

    auto info_string= Array<char>{memory_globals::default_scratch_allocator()};
    if (length) {
      char info_log[length];
      glGetShaderInfoLog(shader_handle, length, nullptr, info_log);
      string::append(info_string, info_log);
    }
    return info_string;
  }

} // namespace shader_program

} // namespace knight
