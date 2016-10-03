// Citation: https://github.com/mosra/magnum/blob/03106cf5cbff3e9b17394a5f7d11bd3cba398e12/src/Magnum/Mesh.h

#pragma once

#include "attribute.h"
#include "shader_types.h"

#include <type_traits>

namespace knight {

class ArrayObject {
 public:
  enum class IndexType {
    UnsignedByte = GL_UNSIGNED_BYTE,
    UnsignedShort = GL_UNSIGNED_SHORT,
    UnsignedInt = GL_UNSIGNED_INT
  };

  enum class Primitive {
    Points = GL_POINTS,
    LineStrip = GL_LINE_STRIP,
    LineLoop = GL_LINE_LOOP,
    Lines = GL_LINES,
    LineStripAdjacency = GL_LINE_STRIP_ADJACENCY,
    LinesAdjacency = GL_LINES_ADJACENCY,
    TriangleStrip = GL_TRIANGLE_STRIP,
    TriangleFan = GL_TRIANGLE_FAN,
    Triangles = GL_TRIANGLES,
    TriangleStripAdjacency = GL_TRIANGLE_STRIP_ADJACENCY,
    TrianglesAdjacency = GL_TRIANGLES_ADJACENCY,
    Patches = GL_PATCHES
  };

  ArrayObject();

  ArrayObject(const ArrayObject &) = delete;
  ArrayObject &operator=(const ArrayObject &) = delete;

  ArrayObject(ArrayObject &&other);
  ArrayObject &operator=(ArrayObject &&other);

  ~ArrayObject();

  GLuint handle() const { return handle_; }

  void bind() const;
  void unbind() const;

  template<typename ...Attributes>
  ArrayObject &add_vertex_buffer(BufferObject &buffer, GLintptr offset, const Attributes&... attributes) {
    add_vertex_buffer_internal(buffer, offset, stride_of_interleaved(attributes...), attributes...);
    return *this;
  }

  ArrayObject &set_count(GLsizei count);
  ArrayObject &set_primitive(Primitive primitive);
  ArrayObject &set_index_buffer(BufferObject &buffer, GLintptr offset, IndexType index_type);

  void draw() const;

private:
  GLuint handle_;
  GLsizei count_;
  Primitive primitive_;

  BufferObject *index_buffer_;
  GLintptr index_offset_;
  IndexType index_type_;

  enum class AttributeKind {
    Generic,
    GenericNormalized,
    Integral,
    Double
  };

  template<typename T, typename ...Args>
  static GLsizei stride_of_interleaved(const Attribute<T> &attribute, const Args&... args) {
    return attribute.size() + stride_of_interleaved(args...);
  }

  template<typename ...Args>
  static GLsizei stride_of_interleaved(GLintptr gap, const Args&... args) {
    return gap + stride_of_interleaved(args...);
  }

  static GLsizei stride_of_interleaved() { return 0; }

  template<typename T, typename ...Attributes>
  void
    add_vertex_buffer_internal(
      BufferObject &buffer, GLintptr offset, GLsizei stride,
      const Attribute<T> &attribute, const Attributes&... attributes) {
    add_vertex_attribute(buffer, attribute, offset, stride);
    add_vertex_buffer_internal(buffer, offset + attribute.size(), stride, attributes...);
  }

  template<typename ...Attributes>
  void
    add_vertex_buffer_internal(
      BufferObject &buffer, GLintptr offset, GLsizei stride,
      GLintptr gap, const Attributes&... attributes) {
    add_vertex_buffer_internal(buffer, offset + gap, stride, attributes...);
  }

  void add_vertex_buffer_internal(BufferObject &, GLintptr, GLsizei) {}

  template<typename T>
  void
    add_vertex_attribute(
      std::enable_if_t<std::is_same<typename Attribute<T>::ScalarType, float>::value, BufferObject &>buffer,
      const Attribute<T> &attribute,
      GLintptr offset,
      GLsizei stride) {
    attribute_pointer(buffer,
        attribute.location(),
        GLint(attribute.components()),
        GLenum(attribute.data_type()),
        attribute.data_option() == Attribute<T>::DataOption::Normalized ? AttributeKind::GenericNormalized : AttributeKind::Generic,
        stride,
        offset);
  }

  template<typename T>
  void
    add_vertex_attribute(
      std::enable_if_t<std::is_integral<typename Attribute<T>::ScalarType>::value, BufferObject &>buffer,
      const Attribute<T> &attribute,
      GLintptr offset,
      GLsizei stride) {
    attribute_pointer(buffer,
        attribute.location(),
        GLint(attribute.components()),
        GLenum(attribute.data_type()),
        AttributeKind::Integral,
        stride,
        offset);
  }

  template<typename T>
  void
    add_vertex_attribute(
      std::enable_if_t<std::is_same<typename Attribute<T>::ScalarType, double>::value, BufferObject &>buffer,
      const Attribute<T> &attribute,
      GLintptr offset,
      GLsizei stride) {
    attribute_pointer(buffer,
        attribute.location(),
        GLint(attribute.components()),
        GLenum(attribute.data_type()),
        AttributeKind::Double,
        stride,
        offset);
  }

  void attribute_pointer(BufferObject &buffer, GLuint location, GLint size, GLenum type,
                        AttributeKind attribute_kind, GLsizei stride, GLintptr offset);
};

bool operator==(const ArrayObject &a, const ArrayObject &b);
bool operator!=(const ArrayObject &a, const ArrayObject &b);
bool operator<(const ArrayObject &a, const ArrayObject &b);
bool operator>(const ArrayObject &a, const ArrayObject &b);

} // namespace knight
