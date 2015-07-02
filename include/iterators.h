#pragma once

#include <hash.h>

namespace foundation {
  template<typename T, uint32_t align>
  T* begin(Array<T, align> &a) { return array::begin(a); }

  template<typename T, uint32_t align>
  const T* begin(const Array<T, align> &a) { return array::begin(a); }

  template<typename T, uint32_t align>
  T* end(Array<T, align> &a) { return array::end(a); }

  template<typename T, uint32_t align>
  const T* end(const Array<T, align> &a) { return array::end(a); }

  template<typename T> const typename Hash<T>::Entry *begin(const Hash<T> &h) { return hash::begin(h); }
  template<typename T> const typename Hash<T>::Entry *end(const Hash<T> &h) { return hash::end(h); }
}
