#pragma once

#include <array.h>
#include <string_stream.h>

namespace knight {

namespace string_util {

  using WideBuffer = foundation::Array<wchar_t, 4>;

  const wchar_t *c_str(WideBuffer &wide_buffer);
  const wchar_t *c_str(WideBuffer &&wide_buffer);

  WideBuffer Widen(foundation::Allocator &allocator, const char *string);
  WideBuffer Widen(foundation::Allocator &allocator, 
                   foundation::string_stream::Buffer &buffer);

  foundation::string_stream::Buffer Narrow(foundation::Allocator &allocator, 
                                           const wchar_t *wide_string);
  foundation::string_stream::Buffer Narrow(foundation::Allocator &allocator,
                                           WideBuffer &wide_buffer);

} // namespace string_util

} // namespace knight
