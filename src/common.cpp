#include "common.h"

#include <logog.hpp>

#include <GL/glew.h>
#include <memory.h>

#include <cerrno>
#include <windows.h>

namespace knight {

const char *glErrorString(GLenum error) {
  switch (error)
  {
    case GL_INVALID_ENUM:
      return "Invalid argument for enum parameter";
    case GL_INVALID_VALUE:
      return "Invalid value for parameter";
    case GL_INVALID_OPERATION:
      return "Invalid operation";
    case GL_OUT_OF_MEMORY:
      return "Out of memory";
  }

  return "ERROR: UNKNOWN ERROR TOKEN";
}

void ExitOnGLError(const char *error_message) {
  const GLenum error_value = glGetError();

  if (error_value != GL_NO_ERROR)
  {
    ERR("%s: %s", error_message, glErrorString(error_value));
    exit(EXIT_FAILURE);
  }
}

void *knight_malloc(size_t size) {
  auto &allocator = foundation::memory_globals::default_allocator();
  return allocator.allocate(size);
}

void knight_free(void *ptr) {
  auto &allocator = foundation::memory_globals::default_allocator();
  allocator.deallocate(ptr);
}

void knight_no_memory() {
  XASSERT(false, "Udp packet failed to allocate");
}

} // namespace knight
