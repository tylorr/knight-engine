#include "platform_types.h"
#include "string_util.h"

#include <string_stream.h>
#include <array.h>

#include <logog.hpp>

using namespace foundation;

namespace knight {

using namespace string_util;

File::File(const char *path) : path_{path} {
  file_handle_ = CreateFile(c_str(Widen(path)), GENERIC_READ | GENERIC_WRITE, 
                            0, nullptr, OPEN_EXISTING, 
                            FILE_ATTRIBUTE_NORMAL, nullptr);

  XASSERT(file_handle_ != INVALID_HANDLE_VALUE, "Failed to open file for RW '%s'", path_);
}

File::File(File &&other) 
    : file_handle_(std::move(other.file_handle_)) {
  other.file_handle_ = INVALID_HANDLE_VALUE;
}

File::~File() {
  if (file_handle_ != INVALID_HANDLE_VALUE) {
    CloseHandle(file_handle_);
    file_handle_ = INVALID_HANDLE_VALUE;
  }
}

void File::Read(foundation::Array<char, 4> &content) const {
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

File &File::operator=(File &&other) {
  file_handle_ = other.file_handle_;
  other.file_handle_ = INVALID_HANDLE_VALUE;
  return *this;
}

} // namespace knight
