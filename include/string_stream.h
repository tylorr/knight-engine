#pragma once

#include "vector.h"

#include <gsl.h>

#include <string>
#include <cstdio>
#include <iterator>

namespace knight {
  /// Functions for operating on an Array<char> as a stream of characters,
  /// useful for string formatting, etc.
  namespace string_stream {
    using Buffer = Vector<char>;

    /// Dumps the item to the stream using a default formatting.
    Buffer & operator<<(Buffer &b, char c);
    Buffer & operator<<(Buffer &b, gsl::czstring<> s);
    Buffer & operator<<(Buffer &b, float f);
    Buffer & operator<<(Buffer &b, int32_t i);
    Buffer & operator<<(Buffer &b, uint32_t i);
    Buffer & operator<<(Buffer &b, uint64_t i);

    /// Uses printf to print formatted data to the stream.
    template<typename... Args>
    Buffer & printf(Buffer &b, gsl::czstring<> format, Args... args);

    /// Pushes the raw data to the stream.
    Buffer & push(Buffer &b, gsl::czstring<> data, uint32_t n);

    /// Pads the stream with spaces until it is aligned at the specified column.
    /// Can be used to column align data. (Assumes each char is 1 space wide,
    /// i.e. does not work with UTF-8 data.)
    Buffer & tab(Buffer &b, uint32_t column);

    /// Adds the specified number of c to the stream.
    Buffer & repeat(Buffer &b, uint32_t count, char c);

    /// Returns the stream as a C-string. There will always be a \0 character
    /// at the end of the returned string. You don't have to explicitly add it
    /// to the buffer.
    gsl::czstring<> c_str(const Buffer &b);
  }

  namespace string_stream_internal {
    using namespace string_stream;

    template <typename T>
    inline Buffer &printf_small(Buffer &b, gsl::czstring<> fmt, const T &t) {
      char s[32];
      snprintf(s, 32, fmt, t);
      return (b << s);
    }
  }

  namespace string_stream {
    inline Buffer & operator<<(Buffer &b, char c) {
      b.push_back(c);
      return b;
    }

    inline Buffer & operator<<(Buffer &b, gsl::czstring<> s) {
      return push(b, s, strlen(s));
    }

    inline Buffer & operator<<(Buffer &b, float f) {
      return string_stream_internal::printf_small(b, "%g", f);
    }

    inline Buffer & operator<<(Buffer &b, int32_t i) {
      return string_stream_internal::printf_small(b, "%d", i);
    }

    inline Buffer & operator<<(Buffer &b, uint32_t i) {
      return string_stream_internal::printf_small(b, "%u", i);
    }

    inline Buffer & operator<<(Buffer &b, uint64_t i) {
      return string_stream_internal::printf_small(b, "%01llx", i);
    }

    template<typename... Args>
    Buffer & printf(Buffer &b, gsl::czstring<> format, Args... args) {
      auto n = snprintf(NULL, 0, format, args...);
      auto end = b.size();

      b.resize(end + n + 1);
      snprintf(b.begin() + end, n + 1, format, args...);
      
      b.resize(end + n);

      return b;
    }

    inline Buffer & push(Buffer &b, gsl::czstring<> data, uint32_t n) {
      auto end = data + n;
      std::copy(data, end, std::back_inserter(b));
      //memcpy(b.begin() + end, data, n);
      return b;
    }

    inline gsl::czstring<> c_str(const Buffer &buffer) {
      auto b = const_cast<Buffer &>(buffer);
      
      // Ensure there is a \0 at the end of the buffer.
      b.push_back('\0');
      b.pop_back();
      return b.data();
    }
  }
}
