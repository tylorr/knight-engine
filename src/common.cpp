#include "common.h"

#include <logog.hpp>

#include <GL/glew.h>
#include <memory.h>

#include <fstream>
#include <cerrno>

using std::string;
using std::ifstream;

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

string GetFileContents(const char *filename) {

  ifstream in(filename, std::ios::in);

  XASSERT(in, "Could not find file '%s'", filename);

  string contents;
  in.seekg(0, std::ios::end);
  contents.resize(in.tellg());
  in.seekg(0, std::ios::beg);
  in.read(&contents[0], contents.size());
  in.close();

  return contents;
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
