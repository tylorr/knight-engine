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

// TODO: TR This is windows only
ReadFileResult ReadEntireFile(foundation::Allocator &allocator, const char *filename) {
  auto result = ReadFileResult{};
  auto file_handle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (file_handle != INVALID_HANDLE_VALUE) {
    LARGE_INTEGER file_size;
    if(GetFileSizeEx(file_handle, &file_size)) {
      XASSERT(file_size.QuadPart <= 0xFFFFFFFF, "Filesize too big for int32");
      uint32_t file_size32 = file_size.LowPart;

      result.content = allocator.allocate(file_size32);
      if (result.content) {
        DWORD bytes_read;
        if(ReadFile(file_handle, result.content, file_size32, &bytes_read, nullptr) &&
           file_size32 == bytes_read) {
          result.content_size = file_size32;
        } else {
          FreeFileMemory(allocator, result.content);
          result.content = nullptr;
        }
      } else {
        ERR("Failed to allocate memory for file read: %s", filename);
      }
    } else {
      ERR("Failed to get file size: %s", filename);
    }
  } else {
    ERR("Failed to open file for reading: %s", filename);
  }
  return result;
}

void FreeFileMemory(foundation::Allocator &allocator, void *file_memory) {
  allocator.deallocate(file_memory);
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
