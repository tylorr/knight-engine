#include "path.h"
#include "string_util.h"

#include <windows.h>

namespace knight {
namespace path {

std::string current_working_directory() {
  auto cwd_buffer_size = GetCurrentDirectoryW(0, nullptr);
  wchar_t cwd_buffer[cwd_buffer_size];

  GetCurrentDirectoryW(cwd_buffer_size, cwd_buffer);

  return string_util::narrow(cwd_buffer);
}

} // namespace path
} // namespace knight
