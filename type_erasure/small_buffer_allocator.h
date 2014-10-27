#pragma once

#include <temp_allocator.h>

template<typename T, int BUFFER_SIZE>
struct SmallBufferAllocator {
 public:
  using value_type = T;

  template<typename U>
  struct rebind { using other = SmallBufferAllocator<U, BUFFER_SIZE>; };

  SmallBufferAllocator(foundation::TempAllocator<BUFFER_SIZE> &allocator) 
    : allocator_(allocator) { }

  template<typename U>
  SmallBufferAllocator(const SmallBufferAllocator<U, BUFFER_SIZE> &other) 
    : allocator_(other.allocator_) { }

  T *allocate(std::size_t n) {
    return static_cast<T *>(allocator_.allocate(n * sizeof(T)));
  }

  void deallocate(T *p, std::size_t n) {
    allocator_.deallocate(p);
  }

  template<typename U>
  friend bool operator==(const SmallBufferAllocator<T, BUFFER_SIZE> &lhs, const SmallBufferAllocator<U, BUFFER_SIZE> &rhs) {
    return &lhs.allocator_ == &rhs.allocator_;
  }

  template<typename U>
  friend bool operator!=(const SmallBufferAllocator<T, BUFFER_SIZE> &lhs, const SmallBufferAllocator<U, BUFFER_SIZE> &rhs) {
    return !(lhs == rhs);
  }

  foundation::TempAllocator<BUFFER_SIZE> &allocator_;
};
