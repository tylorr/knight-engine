#include "program.h"

#include <logog.hpp>

using std::string;

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
    ERR("Failed to link program:\n%s", GetInfoLog().c_str());
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

// void Program::ExtractShaderUniforms() {
//   GLint maxLen, count;
//   glGetProgramiv(handle_, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLen);
//   glGetProgramiv(handle_, GL_ACTIVE_UNIFORMS, &count);

//   GLchar name[maxLen];
//   GLint size, location;
//   GLsizei length;
//   GLenum type;
//   for (int i = 0; i < count; ++i) {
//     glGetActiveUniform(handle_, i, maxLen, &length, &size, &type, name);
//     location = glGetUniformLocation(handle_, name);

//     printf(" %-8d | %s\n", location, name);
//   }
// }

// void Program::NotifyDirty(Uniform *uniform, const GLuint &location) {
//   dirty_uniforms_.push(UniformLocPair(uniform, location));
// }

// void Program::UpdateProgram() {
//   while (!dirty_uniforms_.empty()) {
//     UniformLocPair& top = dirty_uniforms_.front();
//     Uniform *currUniform = top.first;
//     UpdateUniform(currUniform);

//     dirty_uniforms_.pop();
//   }
// }

// void Program::UpdateUniform(Uniform *uniform) const {
//   switch(uniform->type()) {
//     case UniformType::FLOAT_: {
//       UniformFloat *unfiformFloat = static_cast<UniformFloat *>(uniform);
//       unfiformFloat->Update();
//       break;
//     }
//     case UniformType::FLOAT_VEC2: {
//       UniformVec2 *uniformVec2 = static_cast<UniformVec2 *>(uniform);
//       uniformVec2->Update();
//       break;
//     }
//     case UniformType::FLOAT_VEC3: {
//       UniformVec3 *uniformVec3 = static_cast<UniformVec3 *>(uniform);
//       uniformVec3->Update();
//       break;
//     }
//     case UniformType::FLOAT_VEC4: {
//       UniformVec4 *uniformVec4 = static_cast<UniformVec4 *>(uniform);
//       uniformVec4->Update();
//       break;
//     }
//     case UniformType::INT_: {
//       UniformInt *uniformInt = static_cast<UniformInt *>(uniform);
//       uniformInt->Update();
//       break;
//     }

//   }
// }


