#pragma once

#include <hash.h>

namespace foundation {
  template<typename T> T* begin(Array<T> &a) { return array::begin(a); }
  template<typename T> const T* begin(const Array<T> &a) { return array::begin(a); }
  template<typename T> T* end(Array<T> &a) { return array::end(a); }
  template<typename T> const T* end(const Array<T> &a) { return array::end(a); }

  template<typename T> const typename Hash<T>::Entry *begin(const Hash<T> &h) { return hash::begin(h); }
  template<typename T> const typename Hash<T>::Entry *end(const Hash<T> &h) { return hash::end(h); }
}
