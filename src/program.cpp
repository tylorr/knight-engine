#include "program.h"
#include "uniform.h"
#include "uniform_factory.h"

#include <GL/glew.h>

void Program::ExtractShaderUniforms() {
  GLint maxLen, count;
  glGetProgramiv(program_handle_, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLen);
  glGetProgramiv(program_handle_, GL_ACTIVE_UNIFORMS, &count);

  GLchar name[maxLen];
  GLint size, location;
  GLsizei length;
  GLenum type;
  for (int i = 0; i < count; ++i) {
    glGetActiveUniform(program_handle_, i, maxLen, &length, &size, &type, name);
    location = glGetUniformLocation(program_handle_, name);

    printf(" %-8d | %s\n", location, name);
  }
}

void Program::NotifyDirty(Uniform *uniform, unsigned int location) {
  dirty_uniforms_.push(UniformLocPair(uniform, location));
}

void Program::UpdateProgram() {
  while (!dirty_uniforms_.empty()) {
    UniformLocPair& top = dirty_uniforms_.front();
    Uniform *currUniform = top.first;
    UpdateUniform(currUniform);

    dirty_uniforms_.pop();
  }
}

void Program::UpdateUniform(Uniform *uniform) {
  switch(uniform->type()) {
    case UniformType::FLOAT_: {
      UniformFloat *unfiformFloat = static_cast<UniformFloat *>(uniform);
      unfiformFloat->Update();
      break;
    }
    case UniformType::FLOAT_VEC2: {
      UniformVec2 *uniformVec2 = static_cast<UniformVec2 *>(uniform);
      uniformVec2->Update();
      break;
    }
    case UniformType::FLOAT_VEC3: {
      UniformVec3 *uniformVec3 = static_cast<UniformVec3 *>(uniform);
      uniformVec3->Update();
      break;
    }
    case UniformType::FLOAT_VEC4: {
      UniformVec4 *uniformVec4 = static_cast<UniformVec4 *>(uniform);
      uniformVec4->Update();
      break;
    }
    case UniformType::INT_: {
      UniformInt *uniformInt = static_cast<UniformInt *>(uniform);
      uniformInt->Update();
      break;
    }

  }
}
