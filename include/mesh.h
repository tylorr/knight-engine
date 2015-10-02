// Citation: https://github.com/mosra/magnum/blob/03106cf5cbff3e9b17394a5f7d11bd3cba398e12/src/Magnum/Mesh.h

#pragma once

#include "shader_types.h"
#include "attribute.h"

namespace knight {

class Mesh {
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

  Mesh();

  Mesh(const Mesh &) = delete;
  Mesh &operator=(const Mesh &) = delete;

  Mesh(Mesh &&other);
  Mesh &operator=(Mesh &&other);

  ~Mesh();

  GLuint handle() const { return handle_; }

  void Bind() const;
  void Unbind() const;

  template<typename ...Attributes>
  Mesh &AddVertexBuffer(BufferObject &buffer, GLintptr offset, const Attributes&... attributes) {
    AddVertexBufferInternal(buffer, offset, StrideOfInterleaved(attributes...), attributes...);
    return *this;
  }

  Mesh &SetCount(GLsizei count);
  Mesh &SetPrimitive(Primitive primitive);
  Mesh &SetIndexBuffer(BufferObject &buffer, GLintptr offset, IndexType index_type);

  void Draw() const;

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
  static GLsizei StrideOfInterleaved(const Attribute<T> &attribute, const Args&... args) {
    return attribute.size() + StrideOfInterleaved(args...);
  }

  template<typename ...Args>
  static GLsizei StrideOfInterleaved(GLintptr gap, const Args&... args) {
    return gap + StrideOfInterleaved(args...);
  }

  static GLsizei StrideOfInterleaved() { return 0; }

  template<typename T, typename ...Attributes>
  void 
    AddVertexBufferInternal(
      BufferObject &buffer, GLintptr offset, GLsizei stride, 
      const Attribute<T> &attribute, const Attributes&... attributes) {
    AddVertexAttribute(buffer, attribute, offset, stride);
    AddVertexBufferInternal(buffer, offset + attribute.size(), stride, attributes...);
  }

  template<typename ...Attributes>
  void 
    AddVertexBufferInternal(
      BufferObject &buffer, GLintptr offset, GLsizei stride, 
      GLintptr gap, const Attributes&... attributes) {
    AddVertexBufferInternal(buffer, offset + gap, stride, attributes...);
  }

  void AddVertexBufferInternal(BufferObject &, GLintptr, GLsizei) {}

  template<typename T>
  void 
    AddVertexAttribute(
      std::enable_if_t<std::is_same<typename Attribute<T>::ScalarType, float>::value, BufferObject &>buffer, 
      const Attribute<T> &attribute, 
      GLintptr offset, 
      GLsizei stride) {
    AttributePointer(buffer,
        attribute.location(),
        GLint(attribute.components()),
        GLenum(attribute.data_type()),
        attribute.data_option() == Attribute<T>::DataOption::Normalized ? AttributeKind::GenericNormalized : AttributeKind::Generic,
        stride,
        offset);
  }

  template<typename T>
  void 
    AddVertexAttribute(
      std::enable_if_t<std::is_integral<typename Attribute<T>::ScalarType>::value, BufferObject &>buffer, 
      const Attribute<T> &attribute, 
      GLintptr offset, 
      GLsizei stride) {
    AttributePointer(buffer,
        attribute.location(),
        GLint(attribute.components()),
        GLenum(attribute.data_type()),
        AttributeKind::Integral,
        stride,
        offset);
  }

  template<typename T>
  void 
    AddVertexAttribute(
      std::enable_if_t<std::is_same<typename Attribute<T>::ScalarType, double>::value, BufferObject &>buffer, 
      const Attribute<T> &attribute, 
      GLintptr offset, 
      GLsizei stride) {
    AttributePointer(buffer,
        attribute.location(),
        GLint(attribute.components()),
        GLenum(attribute.data_type()),
        AttributeKind::Double,
        stride,
        offset);
  }

  void AttributePointer(BufferObject &buffer, GLuint location, GLint size, GLenum type, 
                        AttributeKind attribute_kind, GLsizei stride, GLintptr offset);
};

} // namespace knight
