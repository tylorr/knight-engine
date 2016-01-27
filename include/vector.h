#pragma once

#include "std_allocator.h"

#include <memory.h>

#include <vector>

namespace knight {

template<typename T>
class Vector : public std::vector<T, StdAllocator<T>> {
 public:
  using std::vector<T, StdAllocator<T>>::vector;
  Vector(foundation::Allocator &allocator)
    : std::vector<T, StdAllocator<T>>{StdAllocator<T>{allocator}} {}
};

} // namespace knight
