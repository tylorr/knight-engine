#include "common.h"

#include <logog.hpp>

#include <GL/glew.h>

namespace knight {

const char *getErrorString(GLenum error) {
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

void ExitOnGLError(const std::string &error_message) {
  const GLenum error_value = glGetError();

  if (error_value != GL_NO_ERROR)
  {
    ERR("%s: %s", error_message.c_str(), getErrorString(error_value));
    exit(EXIT_FAILURE);
  }
}

} // namespace knight
