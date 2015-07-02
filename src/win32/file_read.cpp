#include "platform_types.h"
#include "string_util.h"

#include <string_stream.h>
#include <temp_allocator.h>

#include <logog.hpp>

using namespace foundation;

namespace knight {

using namespace string_util;

FileRead::FileRead(const char *path) : path_{path} {
  TempAllocator128 alloc;
  file_handle_ = CreateFile(c_str(Widen(alloc, path)), GENERIC_READ, 
                            FILE_SHARE_READ, nullptr, OPEN_EXISTING, 
                            FILE_ATTRIBUTE_NORMAL, nullptr);

  XASSERT(file_handle_ != INVALID_HANDLE_VALUE, "Failed to open file for reading '%s'", path_);
}

FileRead::FileRead(FileRead &&other) 
    : file_handle_(std::move(other.file_handle_)) {
  other.file_handle_ = INVALID_HANDLE_VALUE;
}

FileRead::~FileRead() {
  if (file_handle_ != INVALID_HANDLE_VALUE) {
    CloseHandle(file_handle_);
    file_handle_ = INVALID_HANDLE_VALUE;
  }
}

void FileRead::Read(foundation::Array<char, 4> &content) const {
  LARGE_INTEGER file_size;
  auto get_file_size_result = GetFileSizeEx(file_handle_, &file_size);
  XASSERT(get_file_size_result, "Failed to get file size for '%s'", path_);

  XASSERT(file_size.QuadPart <= 0xFFFFFFFF, "Filesize too large for uint32 '%s'", path_);
  auto file_size32 = file_size.LowPart;

  array::resize(content, file_size32);

  DWORD bytes_read;
  auto read_result = ReadFile(file_handle_, array::begin(content), file_size32, &bytes_read, nullptr);

  XASSERT(read_result, "Failed to read file '%s'", path_);
}

FileRead &FileRead::operator=(FileRead &&other) {
  file_handle_ = other.file_handle_;
  other.file_handle_ = INVALID_HANDLE_VALUE;
  return *this;
}

} // namespace knight
