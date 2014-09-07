#include "vertex_array.h"

namespace knight {

VertexArray::~VertexArray() {
  glDeleteVertexArrays(1, &handle_);
}

void VertexArray::Initialize() {
  glGenVertexArrays(1, &handle_);
}

void VertexArray::Bind() const {
  glBindVertexArray(handle_);
}

void VertexArray::Unbind() const {
  glBindVertexArray(0);
}

void VertexArray::BindAttribute(const GLint &attribute, const GLint &size,
                                const GLenum &type, const GLboolean &normalized,
                                const GLsizei &stride, const GLvoid *pointer) {
  glEnableVertexAttribArray(attribute);
  glVertexAttribPointer(attribute, size, type, normalized, stride, pointer);
}

} // namespace knight
