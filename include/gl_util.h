#pragma once

#include <GL/gl3w.h>

namespace knight {

#if defined(DEVELOPMENT)
  #define GL_ASSERT(msg, ...) do {                                  \
    auto error_value = glGetError();                                \
    if (error_value != GL_NO_ERROR) {                               \
      printf("\x1b[1m%s:%d:\x1b[0m ", __FILE__, __LINE__);          \
      printf(msg, ## __VA_ARGS__);                                  \
      printf("\n\n");                                               \
      while (error_value != GL_NO_ERROR) {                          \
        printf("\x1b[31m%s\x1b[0m\n", glErrorString(error_value));  \
        error_value = glGetError();                                 \
      }                                                             \
      printf("\n");                                                 \
      stack_trace();                                                \
      abort();                                                      \
    }                                                               \
  } while (false)

  #define GL(line) do { \
    line; \
    XASSERT(glGetError() == GL_NO_ERROR, "OpenGL error"); \
  } while (false)
#else
  #define GL_ASSERT(msg, ...) ((void)0)
  #define GL(line) line
#endif

inline const char *glErrorString(GLenum error) {
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

struct OpenglVersion {
  int major;
  int minor;
};

} // namespace knight
