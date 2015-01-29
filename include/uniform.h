#pragma once

#include "common.h"
#include "shader_types.h"

#include <logog.hpp>

namespace knight {

class UniformBase {
 public:
  UniformBase(foundation::Allocator &allocator, UniformManager &manager, const char *name)
      : manager_{manager},
        program_locations_{allocator},
        name_{name} { }

  virtual ~UniformBase() { }

  const char *name() const { return name_; }

  void AddShaderProgram(GLuint program_handle, GLint location);

  virtual void Push(GLint location) const = 0;

  void NotifyManager() const;
  GLint GetLocation(GLuint program_handle) const;

  UniformManager &manager_;
  foundation::Hash<GLint> program_locations_;
  const char *name_;
 
 private:
  KNIGHT_DISALLOW_COPY_AND_ASSIGN(UniformBase);
  KNIGHT_DISALLOW_MOVE_AND_ASSIGN(UniformBase);
};

template<typename T, size_t row_count, size_t col_count = 1>
class Uniform : public UniformBase {
 public:
  using Type = T;

  using UniformBase::UniformBase;

  virtual void Push(GLint location) const;
  void SetValue(const T *values);

  static const size_t kElementCount = row_count * col_count;
  static const size_t kElementSize = sizeof(T);
  static const size_t kTotalElementSize = kElementCount * kElementSize;

  T elements_[kElementCount];

 private:
  KNIGHT_DISALLOW_COPY_AND_ASSIGN(Uniform);
  KNIGHT_DISALLOW_MOVE_AND_ASSIGN(Uniform);
};

template<typename T, size_t row_count, size_t col_count>
void Uniform<T, row_count, col_count>::SetValue(const T *values) {
  if (std::memcmp(elements_, values, kTotalElementSize) != 0) {
    std::memcpy(elements_, values, kTotalElementSize);
    NotifyManager();

    //manager_->NotifyDirty()
    //NotifyOwners();
  }
}

} // namespace knight
