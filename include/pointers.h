#pragma once

#include "std_allocator.h"
#include "copyer.h"

#include <memory.h>

namespace knight {

template<typename T>
using pointer = std::unique_ptr<T, StdDeleter>;

template<typename T, typename... Args>
auto allocate_shared(foundation::Allocator &alloc, Args&&... args) {
  return std::allocate_shared<T>(StdAllocator<T>{alloc}, std::forward<Args>(args)...);
}

template<typename T, typename... Args>
auto allocate_unique(foundation::Allocator &allocator, Args&&... args) 
  -> std::enable_if_t<!std::is_array<T>::value, pointer<T>> {
  auto p = allocator.make_new<T>(std::forward<Args>(args)...);
  return pointer<T>{p, StdDeleter{allocator}};
}

template<typename T>
auto allocate_unique(foundation::Allocator &allocator, uint32_t count)
  -> std::enable_if_t<std::is_array<T>::value, pointer<T>> {
  auto arr = allocator.make_array<std::remove_extent_t<T>>(count);
  return pointer<T>{arr, StdDeleter{allocator, count}};
}

// citation: https://turingtester.wordpress.com/2015/06/27/cs-rule-of-zero/

template<typename T>
struct PointerDeepCopier {
  auto operator()(const pointer<T> &other) -> pointer<T> {
    auto &deleter = other.get_deleter();
    return allocate_unique<T>(*deleter.allocator, *other);
  }
};

template<typename T>
using copy_ptr = Copyer<pointer<T>, PointerDeepCopier<T>>;

template<typename T, typename ...Args>
auto allocate_copy(foundation::Allocator &allocator, Args&&... args) {
  return 
    copy_ptr<T>{
      allocate_unique<T>(allocator, std::forward<Args>(args)...)};
}

} // namespace knight
