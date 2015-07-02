#pragma once

#include "std_allocator.h"
#include "copyer.h"

#include <memory.h>

namespace knight {

template<typename T, typename... Args>
auto allocate_shared(foundation::Allocator &alloc, Args&&... args) {
  return std::allocate_shared<T>(StdAllocator<T>{alloc}, std::forward<Args>(args)...);
}

template<typename T, typename Allocator, typename... Args>
auto allocate_unique(const Allocator &alloc, Args&&... args) {
  using Traits = std::allocator_traits<Allocator>;

  static_assert(std::is_same<typename Traits::value_type, std::remove_cv_t<T>>{}(), 
                "Allocator has incorrect value_type");

  Allocator a{alloc};
  auto p = Traits::allocate(a, 1);
  try {
    Traits::construct(a, std::addressof(*p), std::forward<Args>(args)...);
    using D = StdDeleter<Allocator>;
    return std::unique_ptr<T, D>(p, D{a});
  } catch (...) {
    Traits::deallocate(a, p, 1);
    throw;
  }
}

template<typename T, typename... Args>
auto allocate_unique(foundation::Allocator &foundation_allocator, Args&&... args) {
  return allocate_unique<T>(StdAllocator<T>{foundation_allocator}, 
                            std::forward<Args>(args)...);
}

template<typename T>
using pointer = std::unique_ptr<T, ptr_deleter<T>>;

template<typename T>
struct DeepCopyPointer {
  pointer<T> operator()(const pointer<T> &other) {
    auto &deleter = other.get_deleter();
    auto &allocator = *deleter.allocator_.allocator_;
    return allocate_unique<T>(allocator, *other);
  }
};

template<typename T>
using copy_ptr = Copyer<pointer<T>, DeepCopyPointer<T>>;

template<typename T, typename... Args>
auto allocate_copy(foundation::Allocator &foundation_allocator, Args&&... args) {
  return copy_ptr<T>{
    allocate_unique<T>(
      StdAllocator<T>{foundation_allocator}, 
      std::forward<Args>(args)...)
  };
}

} // namespace knight
