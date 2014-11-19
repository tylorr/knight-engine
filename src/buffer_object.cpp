#include "buffer_object.h"

namespace knight {

BufferObject::~BufferObject() {
  if (handle_) {
    glDeleteBuffers(1, &handle_);
  }
}

void BufferObject::Initialize(const GLenum &target) {
  target_ = target;
  glGenBuffers(1, &handle_);
}

void BufferObject::Initialize(const GLenum &target, const GLsizeiptr &size, 
                              const GLvoid *data, const GLenum &usage) {
  target_ = target;
  glGenBuffers(1, &handle_);
  Bind();
  Data(size, data, usage);
  Unbind();
}

void BufferObject::Bind() const {
  XASSERT(handle_, "Trying to bind an uninitialized buffer object");
  glBindBuffer(target_, handle_);
}

void BufferObject::Unbind() const {
  XASSERT(handle_, "Trying to unbind an uninitialized buffer object");

  auto binding_type = GLenum{};

  switch(target_) {
    case GL_ARRAY_BUFFER:
      binding_type = GL_ARRAY_BUFFER_BINDING;
      break;
    case GL_ELEMENT_ARRAY_BUFFER:
      binding_type = GL_ELEMENT_ARRAY_BUFFER_BINDING;
      break;
    case GL_TEXTURE_BUFFER:
      binding_type = GL_TEXTURE_BUFFER_BINDING;
      break;
    case GL_UNIFORM_BUFFER:
      binding_type = GL_UNIFORM_BUFFER_BINDING;
      break;
    default:
      XASSERT(target_ != target_, "Current buffer target not supported");
  }

  auto buffer_binding = GLint{};
  glGetIntegerv(binding_type, &buffer_binding);

  if (handle_ == buffer_binding) {
    glBindBuffer(target_, 0);
  }
}

void BufferObject::Data(const GLsizeiptr &size, const GLvoid *data, const GLenum &usage) {
  glBufferData(target_, size, data, usage);
}

void BufferObject::SubData(const GLintptr &offset, const GLsizeiptr &size, const GLvoid *data) {
  glBufferSubData(target_, offset, size, data);
}

void BufferObject::GetSubData(const GLintptr &offset, const GLsizeiptr &size, GLvoid *data) {
  glGetBufferSubData(target_, offset, size, data);
}

} // namespace knight
