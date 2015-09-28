#pragma once

#include "shader_types.h"

namespace knight {

class VertexArray {
 public:
  VertexArray();

  VertexArray(const VertexArray &) = delete;
  VertexArray &operator=(const VertexArray &) = delete;

  VertexArray(VertexArray &&other);
  VertexArray &operator=(VertexArray &&other);

  ~VertexArray();

  GLuint handle() const { return handle_; }

  void Bind() const;
  void Unbind() const;

  void BindAttribute(BufferObject &buffer, GLint attribute, GLint size, GLenum type, 
                     GLboolean normalized, GLsizei stride, GLintptr offset);

private:
  GLuint handle_;
};

} // namespace knight
