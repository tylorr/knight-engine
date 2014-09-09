#pragma once

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
  explicit UniformBase(const std::string &name) : name_(name) { }

  UniformBase(const std::string &name, ShaderProgram *shader_program,
              const GLint &location);

  virtual ~UniformBase() { }

  const std::string &name() const { return name_; }

  void AddShaderProgram(ShaderProgram *shader_program, const GLint &location);

  virtual void Update(const GLint &location) const = 0;

 protected:
  std::string name_;
  std::vector<std::pair<ShaderProgram *, GLint>> program_locations_;

  void NotifyOwners() const;
 
 private:
  KNIGHT_DISALLOW_COPY_AND_ASSIGN(UniformBase);
  KNIGHT_DISALLOW_MOVE_AND_ASSIGN(UniformBase);
};

template<typename T, size_t row_count, size_t col_count = 1>
class Uniform : public UniformBase {
 public:
  typedef T Type;

  explicit Uniform(const std::string &name) : UniformBase(name) { }

  Uniform(const std::string &name, ShaderProgram *shader_program,
          const GLint &location) 
    : UniformBase(name, shader_program, location) { }

  virtual void Update(const GLint &location) const;

  void SetValue(const T *values);

 private:
  static const size_t kElementCount = row_count * col_count;
  static const size_t kElementSize = sizeof(T);
  static const size_t kTotalElementSize = kElementCount * kElementSize;

  T elements_[kElementCount];

  KNIGHT_DISALLOW_COPY_AND_ASSIGN(Uniform);
  KNIGHT_DISALLOW_MOVE_AND_ASSIGN(Uniform);
};

template<typename T, size_t row_count, size_t col_count>
void Uniform<T, row_count, col_count>::SetValue(const T *values) {
  if (std::memcmp(elements_, values, kTotalElementSize) != 0) {
    std::memcpy(elements_, values, kTotalElementSize);
    NotifyOwners();
  }
}

} // namespace knight
