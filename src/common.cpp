#include "common.h"

#include <memory.h>

namespace knight {

const char *glErrorString(GLenum error) {
  switch (error) {
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
