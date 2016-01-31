#include "buffer_object.h"
#include "common.h"

namespace knight {

using gsl::span;

BufferObject::BufferObject(Target target) :
    target_{target} {
  GL(glGenBuffers(1, &handle_));
}

BufferObject::BufferObject(BufferObject &&other) :
    handle_{other.handle_},
    target_{other.target_} {
  other.handle_ = 0;
}

BufferObject &BufferObject::operator=(BufferObject &&other) {
  handle_ = other.handle_;
  target_ = other.target_;

  other.handle_ = 0;
  return *this;
}

BufferObject::~BufferObject() {
  if (handle_) {
    glDeleteBuffers(1, &handle_);
  }
}

void BufferObject::bind() const {
  XASSERT(handle_, "Trying to bind an uninitialized buffer object");
  glBindBuffer(GLenum(target_), handle_);
}

void BufferObject::unbind() const {
  XASSERT(handle_, "Trying to unbind an uninitialized buffer object");

  auto binding_type = GLenum{};

  switch(GLenum(target_)) {
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
      XASSERT(false, "Current buffer target not supported");
  }

  auto buffer_binding = GLint{};
  glGetIntegerv(binding_type, &buffer_binding);

  if (handle_ == (GLuint)buffer_binding) {
    glBindBuffer(GLenum(target_), 0);
  }
}

void BufferObject::set_data(gsl::span<const gsl::byte> data, Usage usage) {
  bind();
  GL(glBufferData(GLenum(target_), data.size(), data.data(), GLenum(usage)));
}

void BufferObject::set_subdata(GLintptr offset, span<const gsl::byte> data) {
  bind();
  GL(glBufferSubData(GLenum(target_), offset, data.size(), data.data()));
}

} // namespace knight
