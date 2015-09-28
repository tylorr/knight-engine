#pragma once

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

  void BindAttribute(const GLint& attribute,
                     const GLint &size, const GLenum &type,
                     const GLboolean &normalized, const GLsizei &stride,
                     const GLvoid *pointer);

private:
  GLuint handle_;
};

} // namespace knight
