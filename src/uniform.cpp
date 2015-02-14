#include "uniform.h"
#include "iterators.h"
#include "material.h"

#include <hash.h>

using namespace foundation;

namespace knight {

void UniformBase::AddMaterial(const Material &mat, GLint location) {
  hash::set(materials_, location, &mat);
}

void UniformBase::NotifyDirty() {
  for (auto &item : materials_) {
    auto location = item.key;
    auto &material = item.value;
    manager_.MarkDirty(this, *material, location);
  }
}

template<>
void Uniform<float, 1>::Push(GLint location) {
  GL(glUniform1fv(location, 1, elements_));
}

template<>
void Uniform<float, 2>::Push(GLint location) {
  GL(glUniform2fv(location, 1, elements_));
}

template<>
void Uniform<float, 3>::Push(GLint location) {
  GL(glUniform3fv(location, 1, elements_));
}

template<>
void Uniform<float, 4>::Push(GLint location) {
  GL(glUniform4fv(location, 1, elements_));
}

template<>
void Uniform<int, 1>::Push(GLint location) {
  GL(glUniform1iv(location, 1, elements_));
}

template<>
void Uniform<int, 2>::Push(GLint location) {
  GL(glUniform2iv(location, 1, elements_));
}

template<>
void Uniform<int, 3>::Push(GLint location) {
  GL(glUniform3iv(location, 1, elements_));
}

template<>
void Uniform<int, 4>::Push(GLint location) {
  GL(glUniform4iv(location, 1, elements_));
}

template<>
void Uniform<bool, 1>::Push(GLint location) {
  GL(glUniform1i(location, elements_[0]));
}

template<>
void Uniform<bool, 2>::Push(GLint location) {
  GL(glUniform2i(location, elements_[0], elements_[1]));
}

template<>
void Uniform<bool, 3>::Push(GLint location) {
  GL(glUniform3i(location, elements_[0], elements_[1], elements_[2]));
}

template<>
void Uniform<bool, 4>::Push(GLint location) {
  GL(glUniform4i(location, elements_[0], elements_[1], elements_[2], elements_[3]));
}

template<>
void Uniform<float, 2, 2>::Push(GLint location) {
  GL(glUniformMatrix2fv(location, 1, GL_FALSE, elements_));
}

template<>
void Uniform<float, 3, 3>::Push(GLint location) {
  GL(glUniformMatrix3fv(location, 1, GL_FALSE, elements_));
}

template<>
void Uniform<float, 4, 4>::Push(GLint location) {
  GL(glUniformMatrix4fv(location, 1, GL_FALSE, elements_));
}

} // namespace knight
