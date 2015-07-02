#include "platform_types.h"
#include "windows_utils.h"
#include "string_util.h"

#include <string_stream.h>
#include <array.h>
#include <logog.hpp>
#include <temp_allocator.h>

#include <windows.h>
#include <stdio.h>

using namespace foundation;

namespace knight {

using namespace string_util;
using namespace windows;

FileWrite::FileWrite(const char *path) : path_{path} {
  TempAllocator128 alloc;
  file_handle_ = CreateFile(c_str(Widen(alloc, path)), GENERIC_WRITE, 
                            0, nullptr, CREATE_ALWAYS, 
                            FILE_ATTRIBUTE_NORMAL, nullptr);

  XASSERT(file_handle_ != INVALID_HANDLE_VALUE, "Failed to open file for writing '%s'", path_);
}

FileWrite::FileWrite(FileWrite &&other) 
    : path_{},
      file_handle_{INVALID_HANDLE_VALUE} {
  *this = std::move(other);
}

FileWrite &FileWrite::operator=(FileWrite &&other) {
  if (this != &other)
  {
    path_ = std::move(other.path_);
    file_handle_ = std::move(other.file_handle_);
    other.path_ = "";
    other.file_handle_ = INVALID_HANDLE_VALUE;
  }
  return *this;
}

FileWrite::~FileWrite() {
  if (file_handle_ != INVALID_HANDLE_VALUE) {
    CloseHandle(file_handle_);
    file_handle_ = INVALID_HANDLE_VALUE;
  }
}

void FileWrite::Write(const foundation::Array<char, 4> &content) const {
  Write(array::begin(content), array::size(content));
}

void FileWrite::Write(const void *data, uint32_t size) const {
  XASSERT(file_handle_ != INVALID_HANDLE_VALUE, "Failed to open file for writing '%s'", path_);

  DWORD bytes_written;
  auto write_result = WriteFile(file_handle_, data, size, &bytes_written, nullptr);
  
  XASSERT(write_result, "Failed to write file '%s'", path_);
  XASSERT(size == bytes_written, "Failed to write all bytes to file '%s'", path_);
}



} // namespace knight
