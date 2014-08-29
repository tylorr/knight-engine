#include "shader_program.h"
#include "uniform_factory.h"

#include <logog.hpp>

using std::string;

namespace knight {

ShaderProgram::ShaderProgram() {
  handle_ = glCreateProgram();
}

ShaderProgram::ShaderProgram(UniformFactory *uniform_factory, 
                             const Shader &vertex, const Shader &fragment)
    : uniform_factory_(uniform_factory) {
  handle_ = glCreateProgram();
  Attach(vertex);
  Attach(fragment);
  Link();
}

ShaderProgram::~ShaderProgram() {
  glDeleteProgram(handle_);
}

void ShaderProgram::Attach(const Shader &shader) {
  glAttachShader(handle_, shader.handle());
}

void ShaderProgram::Link() {
  GLint res;

  glLinkProgram(handle_);
  glGetProgramiv(handle_, GL_LINK_STATUS, &res);

  if (res == GL_FALSE) {
    ERR("Failed to link program: %s", GetInfoLog().c_str());
  }
}

void ShaderProgram::Bind() const {
  glUseProgram(handle_);
}

string ShaderProgram::GetInfoLog() const {
  GLint length;
  glGetProgramiv(handle_, GL_INFO_LOG_LENGTH, &length);

  if (length) {
    string infoLog(length, 0);
    glGetProgramInfoLog(handle_, length, nullptr, &infoLog[0]);
    return infoLog;
  } else {
    return "";
  }
}

GLint ShaderProgram::GetAttribute(const GLchar *name) {
  return glGetAttribLocation(handle_, name);
}

void ShaderProgram::Update() {
  for (auto it = dirty_uniforms_.begin(); it != dirty_uniforms_.end(); ) {
    auto location = (*it).first;
    auto *uniform = (*it).second;

    DBUG("Updating %s at location %d for program %d", uniform->name().c_str(), location, handle_);

    uniform->Update(location);

    dirty_uniforms_.erase(it++);
  }
}

void ShaderProgram::NotifyDirty(const GLint &location, 
                                const UniformBase *uniform) {
  dirty_uniforms_[location] = uniform;
}

void ShaderProgram::ExtractUniforms() {
  GLint max_uniform_name_length;
  glGetProgramiv(handle_, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_uniform_name_length);

  GLint uniform_count;
  glGetProgramiv(handle_, GL_ACTIVE_UNIFORMS, &uniform_count);

  for (GLint i = 0; i < uniform_count; ++i) {
    char name[max_uniform_name_length];

    GLint value_size;
    GLenum value_type;

    glGetActiveUniform(handle_, i, max_uniform_name_length, nullptr, 
                       &value_size, &value_type, name);

    GLint location = glGetUniformLocation(handle_, name);

    uniform_factory_->Create(this, location, name, value_type);
  }
}

} // namespace knight
