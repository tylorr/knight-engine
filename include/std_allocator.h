#pragma once

#include "common.h"

#include <memory.h>

#include <memory>

namespace knight {

template<typename T>
struct StdAllocator;

struct StdDeleter;

template<typename T>
struct StdAllocator {
  using value_type = T;

  template<typename U>
  struct rebind { using other = StdAllocator<U>; };

  StdAllocator() = default;
  StdAllocator(foundation::Allocator &allocator) : allocator{&allocator} { }
  StdAllocator(const StdAllocator<T> &other) = default;

  template<typename U>
  StdAllocator(const StdAllocator<U> &other)
    : allocator{other.allocator} { }

  T *allocate(std::size_t n) {
    XASSERT(allocator != nullptr, "Cannot allocate without foundation allocator");
    return static_cast<T *>(allocator->allocate(n * sizeof(T)));
  }

  void deallocate(T *p, std::size_t n) {
    allocator->deallocate(p);
  }

  foundation::Allocator *allocator;
};

struct StdDeleter {
  StdDeleter() = default;

  explicit StdDeleter(foundation::Allocator &allocator) :
    allocator{&allocator} {}

  StdDeleter(foundation::Allocator &allocator, uint32_t array_size) :
    allocator{&allocator},
    array_size_{array_size} {}

  template<typename T>
  std::enable_if_t<!std::is_array<T>::value>
    operator()(T *p) const {
    XASSERT(allocator != nullptr, "Cannot delete pointer without foundation allocator");
    allocator->make_delete(p);
  }

  template<typename T>
  std::enable_if_t<std::is_array<T>::value>
    operator()(T *p) const {
    XASSERT(allocator != nullptr, "Cannot delete pointer without foundation allocator");
    allocator->make_array_delete(p, array_size_);
  }

  foundation::Allocator *allocator;
  uint32_t array_size_;
};

template<typename T, typename U>
bool operator==(const StdAllocator<T> &lhs, const StdAllocator<U> &rhs) {
  return &lhs.allocator == &rhs.allocator;
}

template<typename T, typename U>
bool operator!=(const StdAllocator<T> &lhs, const StdAllocator<U> &rhs) {
  return !(lhs == rhs);
}

} // namespace knight
