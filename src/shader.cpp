#include "shader.h"

#include <logog.hpp>

#include <fstream>
#include <streambuf>

using std::string;
using std::ifstream;
using std::istreambuf_iterator;

namespace knight {

void Shader::Initialize(const ShaderType &type, const std::string &code) {
  handle_ = glCreateShader(static_cast<GLenum>(type));
  Source(code);
  Compile();
}

Shader::~Shader() {
  glDeleteShader(handle_);
}

void Shader::Source(const std::string &code) {
  const char *c = code.c_str();
  glShaderSource(handle_, 1, &c, nullptr);
}

void Shader::Compile() {
  GLint res;

  glCompileShader(handle_);
  glGetShaderiv(handle_, GL_COMPILE_STATUS, &res);

  if (res == GL_FALSE) {
    ERR("Failed to compile shader: %s", GetInfoLog().c_str());
  }
}

string Shader::GetInfoLog() const {
  GLint length;
  glGetShaderiv(handle_, GL_INFO_LOG_LENGTH, &length);

  if (length) {
    string infoLog(length, 0);
    glGetShaderInfoLog(handle_, length, nullptr, &infoLog[0]);
    return infoLog;
  } else {
    return "";
  }
}

} // namespace knight
