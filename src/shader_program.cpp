#include "shader_program.h"
#include "uniform_factory.h"

#include <logog.hpp>

using std::string;

namespace knight {

void ShaderProgram::Initialize(UniformFactory &uniform_factory, 
                               const string &source) {
  handle_ = glCreateProgram();
  vertex_handle_ = CreateAndAttachShader(GL_VERTEX_SHADER, source);
  fragment_handle_ = CreateAndAttachShader(GL_FRAGMENT_SHADER, source);
  
  GLint result;
  glLinkProgram(handle_);
  glGetProgramiv(handle_, GL_LINK_STATUS, &result);
  XASSERT(result != GL_FALSE, "Failed to link program: %s", GetProgramInfoLog().c_str());

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

    auto location = glGetUniformLocation(handle_, name);

    uniform_factory.Create(*this, location, name, value_type);
  }
}

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

GLuint ShaderProgram::CreateAndAttachShader(GLenum type, const string &source) {
  auto shader_handle = glCreateShader(type);

  auto type_define = string{"#define "};

  switch(type) {
    case GL_VERTEX_SHADER:
      type_define += "VERTEX";
      break;
    case GL_FRAGMENT_SHADER:
      type_define += "FRAGMENT";
      break;
  }

  type_define += "\n";

  auto source_and_define = (type_define + source).c_str();

  glShaderSource(shader_handle, 1, &source_and_define, nullptr);
  glCompileShader(shader_handle);

  auto res = GLint{};
  glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &res);

  XASSERT(res != GL_FALSE, "Failed to compile shader %d: %s", shader_handle, GetShaderInfoLog(shader_handle).c_str());

  glAttachShader(handle_, shader_handle);

  return shader_handle;
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

GLint ShaderProgram::GetAttributeLocation(const GLchar *name) {
  return glGetAttribLocation(handle_, name);
}

void ShaderProgram::Update() {  
  for (auto it = dirty_uniforms_.begin(); it != dirty_uniforms_.end(); ) {
    auto location = (*it).first;
    auto *uniform = (*it).second;

    uniform->Update(location);

    dirty_uniforms_.erase(it++);
  }
}

void ShaderProgram::NotifyDirty(const GLint &location, 
                                const UniformBase *uniform) {
  dirty_uniforms_[location] = uniform;
}

string ShaderProgram::GetProgramInfoLog() const {
  GLint length;
  glGetProgramiv(handle_, GL_INFO_LOG_LENGTH, &length);

  if (length) {
    auto infoLog = string(length, 0);
    glGetProgramInfoLog(handle_, length, nullptr, &infoLog[0]);
    return infoLog;
  } else {
    return "";
  }
}

string ShaderProgram::GetShaderInfoLog(GLuint shader_handle) const {
  GLint length;
  glGetShaderiv(handle_, GL_INFO_LOG_LENGTH, &length);

  if (length) {
    string infoLog(length, 0);
    glGetShaderInfoLog(shader_handle, length, nullptr, &infoLog[0]);
    return infoLog;
  } else {
    return "";
  }
}

} // namespace knight
