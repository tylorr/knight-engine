#include "windows_utils.h"
#include "string_util.h"

#include <windows.h>

using namespace foundation;
using namespace foundation::string_stream;

namespace knight {

namespace windows {

  void GetLastErrorMessage(Buffer &result) { 
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

    TempAllocator64 alloc;
    auto error_message = string_util::Narrow(alloc, buffer);
    LocalFree(buffer);

    printf(
      result, 
      "%d: %s",
      error_code, 
      string_stream::c_str(error_message));
  }

} // namespace windows
} // namespace knight
