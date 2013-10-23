#include "program.h"

#include <logog.hpp>

using std::string;

namespace knight {

Program::Program() {
  handle_ = glCreateProgram();
}

Program::Program(const Shader &vertex, const Shader &fragment) {
  handle_ = glCreateProgram();
  Attach(vertex);
  Attach(fragment);
  Link();
}

Program::~Program() {
  glDeleteProgram(handle_);
}

void Program::Attach(const Shader &shader) {
  glAttachShader(handle_, shader.handle());
}

void Program::Link() {
  GLint res;

  glLinkProgram(handle_);
  glGetProgramiv(handle_, GL_LINK_STATUS, &res);

  if (res == GL_FALSE) {
    ERR("Failed to link program: %s", GetInfoLog().c_str());
  }
}

void Program::Bind() const {
  glUseProgram(handle_);
}

string Program::GetInfoLog() const {
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

GLint Program::GetAttribute(const GLchar *name) {
  return glGetAttribLocation(handle_, name);
}

} // namespace knight
