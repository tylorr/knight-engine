#ifndef KNIGHT_UNIFORM_H_
#define KNIGHT_UNIFORM_H_

#include "common.h"

#include <logog.hpp>

#include <GL/glew.h>

#include <string>
#include <cstring>
#include <vector>
#include <utility>

namespace knight {

class ShaderProgram;

class UniformBase {
 public:
  UniformBase(const std::string &name, ShaderProgram *shader_program,
              const GLint &location);

  virtual ~UniformBase() { }

  void AddShaderProgram(ShaderProgram *shader_program, const GLint &location);

  virtual void Update(const GLint &location) const = 0;

  const std::string &name() const { return name_; }

 protected:
  std::string name_;
  std::vector<std::pair<ShaderProgram *, GLint>> program_locations_;

  void NotifyOwners() const;
};

template<typename T, size_t row_count, size_t col_count = 1>
class Uniform : public UniformBase {
 public:
  typedef T Type;

  Uniform(const std::string &name, ShaderProgram *shader_program,
              const GLint &location) 
    : UniformBase(name, shader_program, location) {
    std::memset(elements_, 0, element_count_);    
  }

  virtual void Update(const GLint &location) const;

  void SetValue(const T *values);

 private:
  static const size_t element_count_ = row_count * col_count;
  static const size_t element_size_ = sizeof(T);
  static const size_t total_element_size_ = element_count_ * element_size_;

  T elements_[element_count_];
};

template<typename T, size_t row_count, size_t col_count>
void Uniform<T, row_count, col_count>::SetValue(const T *values) {
  if (std::memcmp(elements_, values, total_element_size_) != 0) { 
    std::memcpy(elements_, values, total_element_size_);
    NotifyOwners();
  }
}

} // namespace knight

#endif // KNIGHT_UNIFORM_H_
