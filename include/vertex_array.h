#ifndef KNIGHT_VERTEX_ARRAY_H_
#define KNIGHT_VERTEX_ARRAY_H_

#include "buffer_object.h"

namespace knight {

class VertexArray {
public:
  VertexArray() { }
  ~VertexArray();

  GLuint handle() const { return handle_; }

  void Initialize();

  void Bind() const;
  void Unbind() const;

  void BindAttribute(const GLint& attribute,
                     const GLint &size, const GLenum &type,
                     const GLboolean &normalized, const GLsizei &stride,
                     const GLvoid *pointer);

private:
  GLuint handle_;
};

} // namespace knight

#endif // KNIGHT_VERTEX_ARRAY_H_
