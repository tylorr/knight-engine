#include "windows_utils.h"
#include "string_util.h"

#include <temp_allocator.h>

#include <windows.h>

using namespace foundation;
using namespace foundation::string_stream;

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
    auto error_message = string_util::Narrow(alloc, buffer);
    LocalFree(buffer);

    printf("%lu: %s\n", error_code, string_stream::c_str(error_message));

    // printf(
    //   result, 
    //   "%d: %s",
    //   error_code, 
    //   string_stream::c_str(error_message));
  }

} // namespace windows
} // namespace knight
