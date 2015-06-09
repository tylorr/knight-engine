#include "windows_utils.h"

using namespace foundation;
using namespace foundation::string_stream;

namespace knight {

namespace windows {

using namespace string_util;

Buffer GetLastErrorMessage() { 
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

  auto error_message = Narrow(buffer);
  LocalFree(buffer);

  Buffer result{memory_globals::default_scratch_allocator()};
  printf(
    result, 
    "%d: %s",
    error_code, 
    string_stream::c_str(error_message));

  return result;
}

} 

}
