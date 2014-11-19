#include "shader_program.h"
#include "uniform_factory.h"

#include <logog.hpp>

using std::string;

namespace knight {

void ShaderProgram::Initialize(const Shader &vertex, const Shader &fragment, 
                               UniformFactory &uniform_factory) {
  handle_ = glCreateProgram();
  Attach(vertex);
  Attach(fragment);
  Link();

  ExtractUniforms(uniform_factory);
}

ShaderProgram::~ShaderProgram() {
  if (handle_) {
    glDeleteProgram(handle_);
  }
}

void ShaderProgram::Attach(const Shader &shader) {
  glAttachShader(handle_, shader.handle());
}

void ShaderProgram::Link() {
  GLint result;

  glLinkProgram(handle_);
  glGetProgramiv(handle_, GL_LINK_STATUS, &result);

  XASSERT(result != GL_FALSE, "Failed to link program: %s", GetInfoLog().c_str());
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

void ShaderProgram::ExtractUniforms(UniformFactory &uniform_factory) {
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

string ShaderProgram::GetInfoLog() const {
  auto length = GLint{};
  glGetProgramiv(handle_, GL_INFO_LOG_LENGTH, &length);

  if (length) {
    auto infoLog = string(length, 0);
    glGetProgramInfoLog(handle_, length, nullptr, &infoLog[0]);
    return infoLog;
  } else {
    return "";
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

} // namespace knight
