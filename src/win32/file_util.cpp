#include "file_util.h"
#include "string_util.h"

#include <temp_allocator.h>

#include <windows.h>

namespace knight {

using namespace string_util;

namespace file_util_internal {
  uint64_t UInt64FileTime(FILETIME windows_file_time) {
    ULARGE_INTEGER large_file_time;
    large_file_time.LowPart = windows_file_time.dwLowDateTime;
    large_file_time.HighPart = windows_file_time.dwHighDateTime;

    return large_file_time.QuadPart;
  }
}

namespace file_util {

uint64_t GetLastWriteTime(const char *filename) {
  FILETIME last_write_time;

  TempAllocator128 alloc;
  WIN32_FILE_ATTRIBUTE_DATA data;
  if(GetFileAttributesEx(c_str(Widen(alloc, filename)), GetFileExInfoStandard, &data)) {
    last_write_time = data.ftLastWriteTime;
  }

  return file_util_internal::UInt64FileTime(last_write_time);
}

} // namespace file_util

} // namespace knight
