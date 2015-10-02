#include "mesh.h"
#include "common.h"
#include "buffer_object.h"

namespace knight {

Mesh::Mesh() {
  GL(glGenVertexArrays(1, &handle_));
}

Mesh::Mesh(Mesh &&other) :
    handle_{other.handle_} {
  other.handle_ = 0;
}

Mesh &Mesh::operator=(Mesh &&other) {
  handle_ = other.handle_;
  other.handle_ = 0;
  return *this;
}

Mesh::~Mesh() {
  if (handle_) {
    glDeleteVertexArrays(1, &handle_);
  }
}

void Mesh::Bind() const {
  XASSERT(handle_, "Trying to bind an uninitialized vertex array");
  GL(glBindVertexArray(handle_));
}

void Mesh::Unbind() const {
  auto vertex_array_binding = GLint{};
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vertex_array_binding);
  if (handle_ == (GLuint)vertex_array_binding) {
    glBindVertexArray(0);
  }
}

Mesh &Mesh::SetCount(GLsizei count) {
  count_ = count;
  return *this;
}

Mesh &Mesh::SetPrimitive(Primitive primitive) {
  primitive_ = primitive;
  return *this;
}

Mesh &
  Mesh::SetIndexBuffer(
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

void Mesh::Draw() const {
  Bind();

  if (index_buffer_ == nullptr) {
    glDrawArrays(GLenum(primitive_), 0, count_);
  } else {
    glDrawElements(GLenum(primitive_), count_, GLenum(index_type_), reinterpret_cast<GLvoid *>(index_offset_));
  }
}
 
void Mesh::AttributePointer(BufferObject &buffer, GLuint location, 
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

bool operator==(const Mesh &a, const Mesh &b) {
  return a.handle() == b.handle();
}

bool operator!=(const Mesh &a, const Mesh &b) {
  return !(a == b);
}

bool operator<(const Mesh &a, const Mesh &b) {
  return a.handle() < b.handle();
}

bool operator>(const Mesh &a, const Mesh &b) {
  return a.handle() > b.handle();
}

} // namespace knight
