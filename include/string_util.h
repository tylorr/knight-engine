#pragma once

#include <array.h>
#include <string_stream.h>

namespace knight {

namespace string_util {

  using WideBuffer = foundation::Array<wchar_t, 4>;

  const wchar_t *c_str(WideBuffer &wide_buffer);
  const wchar_t *c_str(WideBuffer &&wide_buffer);

  WideBuffer Widen(const char *string);
  WideBuffer Widen(foundation::string_stream::Buffer &buffer);

  foundation::string_stream::Buffer Narrow(const wchar_t *wide_string);
  foundation::string_stream::Buffer Narrow(WideBuffer &wide_buffer);

} // namespace string_util

} // namespace knight
