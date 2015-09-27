#pragma once

namespace knight {

class BufferObject {
 public:
  BufferObject(GLenum target_);

  BufferObject(const BufferObject &) = delete;
  BufferObject &operator=(const BufferObject &) = delete;

  BufferObject(BufferObject &&other);
  BufferObject &operator=(BufferObject &&other);

  ~BufferObject();

  GLuint handle() const { return handle_; }
  GLenum target() const { return target_; }

  void Initialize(const GLsizeiptr &size, const GLvoid *data, const GLenum &usage);

  void Bind() const;
  void Unbind() const;

  void Data(const GLsizeiptr &size, const GLvoid *data, const GLenum &usage);
  void SubData(const GLintptr &offset, const GLsizeiptr &size, const GLvoid *data);

  void GetSubData(const GLintptr &offset, const GLsizeiptr &size, GLvoid *data);

 private:

  GLuint handle_;
  GLenum target_;
};

} // namespace knight
