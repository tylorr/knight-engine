#pragma once

#include "std_allocator.h"

#include <memory.h>
#include <hash.h>
#include <vector>

namespace knight {

template<typename T>
class Vector : public std::vector<T, StdAllocator<T>> {
 public:
  Vector(foundation::Allocator &allocator)
    : std::vector<T, StdAllocator<T>>{StdAllocator<T>{allocator}} {}
};

} // namespace knight

namespace foundation {
namespace multi_hash {

template<typename T>
void get(const Hash<T> &h, uint64_t key, knight::Vector<T> &items) {
  const auto *e = find_first(h, key);
  while (e) {
    items.push_back(e->value);
    e = find_next(h, e);
  }
}

} // namespace multi_hash
} // namespace foundation
