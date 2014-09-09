#pragma once

#include "common.h"
#include "buffer_object.h"

namespace knight {

class VertexArray {
public:
  VertexArray() { }
  VertexArray(VertexArray &&other) : handle_(std::move(other.handle_)) { }
  ~VertexArray();

  GLuint handle() const { return handle_; }

  void Initialize();

  void Bind() const;
  void Unbind() const;

  void BindAttribute(const GLint& attribute,
                     const GLint &size, const GLenum &type,
                     const GLboolean &normalized, const GLsizei &stride,
                     const GLvoid *pointer);

  VertexArray &operator=(VertexArray &&other) {
    handle_ = std::move(other.handle_);
    return *this;
  }

private:
  GLuint handle_;

  KNIGHT_DISALLOW_COPY_AND_ASSIGN(VertexArray);
};

} // namespace knight
