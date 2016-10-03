#include "array_object.h"
#include "common.h"
#include "gl_util.h"
#include "buffer_object.h"

namespace knight {

ArrayObject::ArrayObject() {
  GL(glGenVertexArrays(1, &handle_));
}

ArrayObject::ArrayObject(ArrayObject &&other) :
    handle_{other.handle_} {
  other.handle_ = 0;
}

ArrayObject &ArrayObject::operator=(ArrayObject &&other) {
  handle_ = other.handle_;
  other.handle_ = 0;
  return *this;
}

ArrayObject::~ArrayObject() {
  if (handle_) {
    glDeleteVertexArrays(1, &handle_);
  }
}

void ArrayObject::bind() const {
  XASSERT(handle_, "Trying to bind an uninitialized vertex array");
  GL(glBindVertexArray(handle_));
}

void ArrayObject::unbind() const {
  auto vertex_array_binding = GLint{};
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vertex_array_binding);
  if (handle_ == (GLuint)vertex_array_binding) {
    glBindVertexArray(0);
  }
}

ArrayObject &ArrayObject::set_count(GLsizei count) {
  count_ = count;
  return *this;
}

ArrayObject &ArrayObject::set_primitive(Primitive primitive) {
  primitive_ = primitive;
  return *this;
}

ArrayObject &
  ArrayObject::set_index_buffer(
    BufferObject &index_buffer, GLintptr offset, IndexType index_type) {

  XASSERT(index_buffer.target() == BufferObject::Target::ElementArray, "Only element arrays can be set as index buffers");

  index_buffer_ = &index_buffer;
  index_offset_ = offset;
  index_type_ = index_type;

  // Bind index buffer to VAO
  bind();
  index_buffer.bind();

  return *this;
}

void ArrayObject::draw() const {
  bind();

  if (index_buffer_ == nullptr) {
    glDrawArrays(GLenum(primitive_), 0, count_);
  } else {
    glDrawElements(GLenum(primitive_), count_, GLenum(index_type_), reinterpret_cast<GLvoid *>(index_offset_));
  }
}

void ArrayObject::attribute_pointer(BufferObject &buffer, GLuint location,
                                   GLint size, GLenum type, AttributeKind attribute_kind,
                                   GLsizei stride, GLintptr offset) {
  bind();
  GL(glEnableVertexAttribArray(location));
  buffer.bind();

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

bool operator==(const ArrayObject &a, const ArrayObject &b) {
  return a.handle() == b.handle();
}

bool operator!=(const ArrayObject &a, const ArrayObject &b) {
  return !(a == b);
}

bool operator<(const ArrayObject &a, const ArrayObject &b) {
  return a.handle() < b.handle();
}

bool operator>(const ArrayObject &a, const ArrayObject &b) {
  return a.handle() > b.handle();
}

} // namespace knight
