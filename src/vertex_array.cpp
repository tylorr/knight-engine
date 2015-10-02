#include "vertex_array.h"
#include "common.h"
#include "buffer_object.h"

namespace knight {

VertexArray::VertexArray() {
  GL(glGenVertexArrays(1, &handle_));
}

VertexArray::VertexArray(VertexArray &&other) :
    handle_{other.handle_} {
  other.handle_ = 0;
}

VertexArray &VertexArray::operator=(VertexArray &&other) {
  handle_ = other.handle_;
  other.handle_ = 0;
  return *this;
}

VertexArray::~VertexArray() {
  if (handle_) {
    glDeleteVertexArrays(1, &handle_);
  }
}

void VertexArray::Bind() const {
  XASSERT(handle_, "Trying to bind an uninitialized vertex array");
  GL(glBindVertexArray(handle_));
}

void VertexArray::Unbind() const {
  auto vertex_array_binding = GLint{};
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vertex_array_binding);
  if (handle_ == (GLuint)vertex_array_binding) {
    glBindVertexArray(0);
  }
}

VertexArray &VertexArray::SetCount(GLsizei count) {
  count_ = count;
  return *this;
}

VertexArray &VertexArray::SetPrimitive(Primitive primitive) {
  primitive_ = primitive;
  return *this;
}

VertexArray &
  VertexArray::SetIndexBuffer(
    BufferObject &index_buffer, GLintptr offset, IndexType index_type) {

  XASSERT(index_buffer.target() == BufferObject::Target::ElementArray, "Only element arrays can be set as index buffers");

  index_buffer_ = &index_buffer;
  index_offset_ = offset;
  index_type_ = index_type;

  // Bind index buffer to VAO
  Bind();
  index_buffer.Bind();

  return *this;
}

void VertexArray::Draw() const {
  Bind();

  if (index_buffer_ == nullptr) {
    glDrawArrays(GLenum(primitive_), 0, count_);
  } else {
    glDrawElements(GLenum(primitive_), count_, GLenum(index_type_), reinterpret_cast<GLvoid *>(index_offset_));
  }
}
 
void VertexArray::AttributePointer(BufferObject &buffer, GLuint location, 
                                   GLint size, GLenum type, AttributeKind attribute_kind, 
                                   GLsizei stride, GLintptr offset) {
  Bind();
  GL(glEnableVertexAttribArray(location));
  buffer.Bind();

  auto offset_ptr = reinterpret_cast<const GLvoid *>(offset);

  switch(attribute_kind) {
    case AttributeKind::Integral:
      GL(glVertexAttribIPointer(location, size, type, stride, offset_ptr));
      break;
    case AttributeKind::Double:
      GL(glVertexAttribLPointer(location, size, type, stride, offset_ptr));
      break;
    default:
      GL(glVertexAttribPointer(location, size, type, attribute_kind == AttributeKind::GenericNormalized, stride, offset_ptr));
      break;
  }
  
}

} // namespace knight
