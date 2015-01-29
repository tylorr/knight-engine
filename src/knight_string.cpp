#include "knight_string.h"

#include <array.h>
#include <murmur_hash.h>
#include <logog.hpp>

#include <cstring>

namespace knight {
namespace string {

void append(foundation::Array<char> &string, const char *c_string) {
  auto new_char_count = strlen(c_string);
  auto original_size = foundation::array::size(string);
  foundation::array::reserve(string, original_size + new_char_count);

  if (original_size > 0) {
    // remove \0
    foundation::array::pop_back(string);
  }

  for (auto i = 0; i < new_char_count; ++i) {
    foundation::array::push_back(string, c_string[i]);
  }

  foundation::array::push_back(string, '\0');
}

void append(foundation::Array<char> &string, char c) {
  if (!foundation::array::empty(string)) {
    // remove \0
    foundation::array::pop_back(string);
  }

  foundation::array::push_back(string, c);
  foundation::array::push_back(string, '\0');
}

size_t length(const foundation::Array<char> &string) {
  auto size = foundation::array::size(string);

  // don't include \0
  if (size > 0) {
    size--;
  }

  return size;
}

const char *c_str(const foundation::Array<char> &string) {
  if (!foundation::array::empty(string)) {
    return foundation::array::begin(string);
  } else {
    return "";
  }
}

uint64_t hash(const char *string, uint64_t seed) {
  return foundation::murmur_hash_64(string, strlen(string), seed);
}

}
}
