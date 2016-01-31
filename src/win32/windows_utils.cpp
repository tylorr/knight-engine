#include "windows_utils.h"
#include "string_util.h"

#include <temp_allocator.h>

#include <windows.h>
#include <cstdio>

using namespace foundation;

namespace knight {

namespace windows {

  void GetLastErrorMessage() { 
    wchar_t *buffer;
    auto error_code = GetLastError();

    FormatMessage(
      FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
      nullptr,
      error_code,
      0,
      (wchar_t *)&buffer,
      0,
      nullptr);

    foundation::TempAllocator64 alloc;
    auto error_message = string_util::narrow(buffer);
    LocalFree(buffer);

    std::printf("%lu: %s\n", error_code, error_message.c_str());

    // printf(
    //   result, 
    //   "%d: %s",
    //   error_code, 
    //   string_stream::c_str(error_message));
  }

} // namespace windows
} // namespace knight
