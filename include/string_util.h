#pragma once

#include <utf8.h>

#include <string>
#include <locale>
#include <codecvt>
#include <iterator>

namespace knight {
  namespace string_util {
    inline std::wstring widen(const std::string &str) {
      // using codecvt = std::codecvt_utf8_utf16<wchar_t>;
      // std::wstring_convert<codecvt> converter;
      // return converter.from_bytes(str);
      std::wstring wide_buffer;
      utf8::utf8to16(str.begin(), str.end(), back_inserter(wide_buffer));
      return wide_buffer;
    }

    inline std::string narrow(const std::wstring &string) {
      using codecvt = std::codecvt_utf8_utf16<wchar_t>;
      std::wstring_convert<codecvt, wchar_t> converter;
      return converter.to_bytes(string);
    }
  }
}

