#ifndef VERTEX_ARRAY_H_
#define VERTEX_ARRAY_H_

#include "buffer_object.h"

namespace knight {

class VertexArray {
public:
  VertexArray();
  ~VertexArray();

  GLuint handle() const { return handle_; }

  void Bind() const;

  void BindBuffer(const BufferObject& buffer);

  void BindAttribute(const BufferObject& buffer, const GLint& attribute,
                     const GLint &size, const GLenum &type,
                     const GLboolean &normalized, const GLsizei &stride,
                     const GLvoid *pointer);

private:
  GLuint handle_;
};

} // namespace knight

#endif
