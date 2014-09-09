#pragma once

#include "common.h"

#include <GL/glew.h>

namespace knight {

class BufferObject {
 public:
  BufferObject() { }
  BufferObject(BufferObject &&other)
    : handle_(std::move(other.handle_)),
      target_(std::move(other.target_)) { }

  ~BufferObject();

  GLuint handle() const { return handle_; }
  GLenum target() const { return target_; }

  void Initialize(const GLenum &target);
  void Initialize(const GLenum &target, const GLsizeiptr &size, 
                  const GLvoid *data, const GLenum &usage);

  void Bind() const;
  void Unbind() const;

  void Data(const GLsizeiptr &size, const GLvoid *data, const GLenum &usage);
  void SubData(const GLintptr &offset, const GLsizeiptr &size, const GLvoid *data);

  void GetSubData(const GLintptr &offset, const GLsizeiptr &size, GLvoid *data);

  BufferObject &operator=(BufferObject &&other) {
    handle_ = std::move(other.handle_);
    target_ = std::move(other.target_);
    return *this;
  }

 private:
  GLuint handle_;
  GLenum target_;

  KNIGHT_DISALLOW_COPY_AND_ASSIGN(BufferObject);
};

} // namespace knight
