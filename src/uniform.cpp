#include "uniform.h"
#include "shader_program.h"

namespace knight {

UniformBase::UniformBase(const std::string &name, ShaderProgram *shader_program,
                         const GLint &location) : name_(name) {
  program_locations_.push_back(std::make_pair(shader_program, location));
}

void UniformBase::AddShaderProgram(ShaderProgram *shader_program, 
                                   const GLint &location) {
  program_locations_.push_back(std::make_pair(shader_program, location));
}

void UniformBase::NotifyOwners() const {
  for (auto program_location : program_locations_) {
    program_location.first->NotifyDirty(program_location.second, this);
  }
}

template<>
void Uniform<float, 1>::Update(const GLint &location) const {
  glUniform1fv(location, 1, elements_);
}

template<>
void Uniform<float, 2>::Update(const GLint &location) const {
  glUniform2fv(location, 1, elements_);
}

template<>
void Uniform<float, 3>::Update(const GLint &location) const {
  glUniform3fv(location, 1, elements_);
}

template<>
void Uniform<float, 4>::Update(const GLint &location) const {
  glUniform4fv(location, 1, elements_);
}

template<>
void Uniform<int, 1>::Update(const GLint &location) const {
  glUniform1iv(location, 1, elements_);
}

template<>
void Uniform<int, 2>::Update(const GLint &location) const {
  glUniform2iv(location, 1, elements_);
}

template<>
void Uniform<int, 3>::Update(const GLint &location) const {
  glUniform3iv(location, 1, elements_);
}

template<>
void Uniform<int, 4>::Update(const GLint &location) const {
  glUniform4iv(location, 1, elements_);
}

template<>
void Uniform<bool, 1>::Update(const GLint &location) const {
  glUniform1i(location, elements_[0]);
}

template<>
void Uniform<bool, 2>::Update(const GLint &location) const {
  glUniform2i(location, elements_[0], elements_[1]);
}

template<>
void Uniform<bool, 3>::Update(const GLint &location) const {
  glUniform3i(location, elements_[0], elements_[1], elements_[2]);
}

template<>
void Uniform<bool, 4>::Update(const GLint &location) const {
  glUniform4i(location, elements_[0], elements_[1], elements_[2], elements_[3]);
}

template<>
void Uniform<float, 2, 2>::Update(const GLint &location) const {
  glUniformMatrix2fv(location, 1, GL_FALSE, elements_);
}

template<>
void Uniform<float, 3, 3>::Update(const GLint &location) const {
  glUniformMatrix3fv(location, 1, GL_FALSE, elements_);
}

template<>
void Uniform<float, 4, 4>::Update(const GLint &location) const {
  glUniformMatrix4fv(location, 1, GL_FALSE, elements_);
}

} // namespace knight
