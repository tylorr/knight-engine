#include "buffer_object.h"

namespace knight {

BufferObject::BufferObject(GLenum target)
    : target_(target) {
  glGenBuffers(1, &handle_);
}

BufferObject::BufferObject(GLenum target, const GLsizeiptr &size, const GLvoid *data, const GLenum &usage)
    : target_(target) {
  glGenBuffers(1, &handle_);
  Data(size, data, usage);
}

BufferObject::~BufferObject() {
  glDeleteBuffers(1, &handle_);
}

void BufferObject::Data(const GLsizeiptr &size, const GLvoid *data, const GLenum &usage) {
  glBindBuffer(target_, handle_);
  glBufferData(target_, size, data, usage);
}

void BufferObject::SubData(const GLintptr &offset, const GLsizeiptr &size, const GLvoid *data) {
  glBindBuffer(target_, handle_);
  glBufferSubData(target_, offset, size, data);
}

void BufferObject::GetSubData(const GLintptr &offset, const GLsizeiptr &size, GLvoid *data) {
  glBindBuffer(target_, handle_);
  glGetBufferSubData(target_, offset, size, data);
}

}; // namespace knight
