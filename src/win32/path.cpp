#include "path.h"
#include "string_util.h"

#include <temp_allocator.h>
#include <array.h>

#include <logog.hpp>

#include <windows.h>

using namespace foundation;
using namespace foundation::string_stream;

namespace knight {

using namespace string_util;

namespace path {

void CurrentWorkingDirectory(Buffer &cwd)
{
  TempAllocator128 allocator;
  WideBuffer wide_working_directory{allocator};

  auto cwd_buffer_size = GetCurrentDirectoryW(0, nullptr);

  array::resize(wide_working_directory, cwd_buffer_size);
  GetCurrentDirectoryW(cwd_buffer_size, array::begin(wide_working_directory));

  cwd = Narrow(*cwd._allocator, wide_working_directory);
}

} // namespace path
} // namespace knight
