#include "string_util.h"
#include "back_inserter.h"

#include <memory.h>
#include <utf8.h>

#include <logog.hpp>

using namespace foundation;
using namespace foundation::string_stream;

namespace knight {

namespace string_util {

  const wchar_t *c_str(WideBuffer &wide_buffer) {
    array::push_back(wide_buffer, L'\0');
    array::pop_back(wide_buffer);
    return array::begin(wide_buffer);
  }

  const wchar_t *c_str(WideBuffer &&wide_buffer) {
    array::push_back(wide_buffer, L'\0');
    array::pop_back(wide_buffer);
    return array::begin(wide_buffer);
  }

  WideBuffer Widen(const char *string) {
    WideBuffer wide_buffer{memory_globals::default_scratch_allocator()};
    utf8::utf8to16(string, string + strlen(string), BackInserter(wide_buffer));
    return wide_buffer;
  }

  WideBuffer Widen(Buffer &buffer) {
    return Widen(string_stream::c_str(buffer));
  }

  Buffer Narrow(const wchar_t *wide_string) {
    Buffer buffer{memory_globals::default_scratch_allocator()};
    utf8::utf16to8(wide_string, wide_string + wcslen(wide_string), BackInserter(buffer));
    return buffer;
  }

  Buffer Narrow(WideBuffer &wide_buffer) {
    return Narrow(c_str(wide_buffer));
  }

} // namespace string_util

} // namespace knight
