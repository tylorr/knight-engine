#pragma once

#include <array.h>

namespace knight {

template<typename T, uint32_t align>
class ArrayBackInsterter {
 public:
  explicit ArrayBackInsterter(foundation::Array<T, align> &array) 
      : array_{std::addressof(array)} { }

  ArrayBackInsterter<T, align> &operator=(const T &value) {
    foundation::array::push_back(*array_, value);
    return *this;
  }

  ArrayBackInsterter<T, align> &operator=(T &&value) {
    foundation::array::push_back(*array_, std::move(value));
    return *this;
  }

  ArrayBackInsterter<T, align> &operator*() {
    return *this;
  }

  ArrayBackInsterter<T, align> &operator++() {
    return *this;
  }

  ArrayBackInsterter<T, align> &operator++(int) {
    return *this;
  }

 private:
  foundation::Array<T, align> *array_;
};

template<typename T, uint32_t align>
ArrayBackInsterter<T, align> BackInserter(foundation::Array<T, align> &array) {
  return ArrayBackInsterter<T, align>(array);
}

} // namespace knight
