#ifndef KNIGHT_BUFFER_OBJECT_H_
#define KNIGHT_BUFFER_OBJECT_H_

#include "common.h"

#include <GL/glew.h>

namespace knight {

class BufferObject {
 public:
  BufferObject(GLenum target);
  BufferObject(GLenum target, const GLsizeiptr &size, const GLvoid *data, const GLenum &usage);

  ~BufferObject();

  GLuint handle() const { return handle_; }
  GLenum target() const { return target_; }

  void Data(const GLsizeiptr &size, const GLvoid *data, const GLenum &usage);
  void SubData(const GLintptr &offset, const GLsizeiptr &size, const GLvoid *data);

  void GetSubData(const GLintptr &offset, const GLsizeiptr &size, GLvoid *data);

 private:
  GLuint handle_;
  GLenum target_;

  KNIGHT_DISALLOW_COPY_AND_ASSIGN(BufferObject);
};

} // namespace knight

#endif // KNIGHT_BUFFER_OBJECT_H_
