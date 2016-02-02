#include "windows_util.h"
#include "string_util.h"

#include <windows.h>

namespace knight {
namespace windows {

std::tuple<std::string, uint32_t> get_last_error() { 
  wchar_t *error_message_buffer;
  auto error_code = GetLastError();

  FormatMessageW(
    FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
    nullptr,
    error_code,
    0,
    reinterpret_cast<wchar_t *>(&error_message_buffer),
    0,
    nullptr);

  auto error_message = string_util::narrow(error_message_buffer);
  LocalFree(error_message_buffer);

  return std::make_tuple(std::move(error_message), error_code);
}

} // namespace windows
} // namespace knight
