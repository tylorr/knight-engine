#include "vertex_array.h"

namespace knight {

VertexArray::VertexArray() {
  glGenVertexArrays(1, &handle_);
}

VertexArray::~VertexArray() {
  glDeleteVertexArrays(1, &handle_);
}

void VertexArray::Bind() const {
  glBindVertexArray(handle_);
}

void VertexArray::BindBuffer(const BufferObject &buffer) {
  Bind();
  glBindBuffer(buffer.target(), buffer.handle());
}

void VertexArray::BindAttribute(const BufferObject& buffer,
                                const GLint &attribute, const GLint &size,
                                const GLenum &type, const GLboolean &normalized,
                                const GLsizei &stride, const GLvoid *pointer) {
  BindBuffer(buffer);
  glEnableVertexAttribArray(attribute);
  glVertexAttribPointer(attribute, size, type, normalized, stride, pointer);
}

}; // namespace knight
