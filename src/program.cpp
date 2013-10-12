#include "program.h"
#include "uniform.h"
#include "uniform_factory.h"

GC Program::gc;

Program::Program() {
  handle_ = Program::gc.Create(glCreateProgram(), glDeleteProgram);
}

Program::Program(const Shader &vertex, const Shader &fragment) {
  handle_ = Program::gc.Create(glCreateProgram(), glDeleteProgram);
  Attach(vertex);
  Attach(fragment);
  Link();
}

Program::~Program() {
  Program::gc.Destroy(handle_);
}

void Program::Attach(const Shader &shader) {
  glAttachShader(handle_, shader.handle());
}

void Program::Link() {
  GLint res;

  glLinkProgram(handle_);
  glGetProgramiv(handle_, GL_LINK_STATUS, &res);

  if (res == GL_FALSE) {
    throw LinkException(GetInfoLog());
  }
}

void Program::Bind() const {
  glUseProgram(handle_);
}

std::string Program::GetInfoLog() const {
  GLint res;
  glGetProgramiv(handle_, GL_INFO_LOG_LENGTH, &res);

  if (res > 0) {
    std::string infoLog(res, 0);
    glGetProgramInfoLog(handle_, res, &res, &infoLog[0]);
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


