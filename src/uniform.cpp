#include "uniform.h"

#include <hash.h>

namespace knight {

void UniformBase::AddShaderProgram(GLuint program_handle, GLint location) {
  foundation::hash::set(program_locations_, program_handle, location);
}

void UniformBase::NotifyManager() const {
  auto end = foundation::hash::end(program_locations_);
  for (auto it = foundation::hash::begin(program_locations_); it != end; ++it) {
    manager_.NotifyDirty(it->key, this);
  }
}

GLint UniformBase::GetLocation(GLuint program_handle) const {
  return foundation::hash::get(program_locations_, program_handle, 0);
}

template<>
void Uniform<float, 1>::Push(GLint location) const {
  glUniform1fv(location, 1, elements_);
}

template<>
void Uniform<float, 2>::Push(GLint location) const {
  glUniform2fv(location, 1, elements_);
}

template<>
void Uniform<float, 3>::Push(GLint location) const {
  glUniform3fv(location, 1, elements_);
}

template<>
void Uniform<float, 4>::Push(GLint location) const {
  glUniform4fv(location, 1, elements_);
}

template<>
void Uniform<int, 1>::Push(GLint location) const {
  glUniform1iv(location, 1, elements_);
}

template<>
void Uniform<int, 2>::Push(GLint location) const {
  glUniform2iv(location, 1, elements_);
}

template<>
void Uniform<int, 3>::Push(GLint location) const {
  glUniform3iv(location, 1, elements_);
}

template<>
void Uniform<int, 4>::Push(GLint location) const {
  glUniform4iv(location, 1, elements_);
}

template<>
void Uniform<bool, 1>::Push(GLint location) const {
  glUniform1i(location, elements_[0]);
}

template<>
void Uniform<bool, 2>::Push(GLint location) const {
  glUniform2i(location, elements_[0], elements_[1]);
}

template<>
void Uniform<bool, 3>::Push(GLint location) const {
  glUniform3i(location, elements_[0], elements_[1], elements_[2]);
}

template<>
void Uniform<bool, 4>::Push(GLint location) const {
  glUniform4i(location, elements_[0], elements_[1], elements_[2], elements_[3]);
}

template<>
void Uniform<float, 2, 2>::Push(GLint location) const {
  glUniformMatrix2fv(location, 1, GL_FALSE, elements_);
}

template<>
void Uniform<float, 3, 3>::Push(GLint location) const {
  glUniformMatrix3fv(location, 1, GL_FALSE, elements_);
}

template<>
void Uniform<float, 4, 4>::Push(GLint location) const {
  glUniformMatrix4fv(location, 1, GL_FALSE, elements_);
}

} // namespace knight
