#pragma once

#include <memory>

namespace knight {

template<typename T>
struct StdAllocator;

template<typename T>
struct StdDeleter;

template<typename T>
using ptr_deleter = StdDeleter<StdAllocator<T>>;

template<typename T>
struct StdAllocator {
  using value_type = T;

  template<typename U>
  struct rebind { using other = StdAllocator<U>; };

  StdAllocator() : allocator_{} { }

  StdAllocator(foundation::Allocator &allocator) 
    : allocator_{&allocator} { }

  StdAllocator(const StdAllocator<T> &other)
    : allocator_{other.allocator_} { }

  template<typename U>
  StdAllocator(const StdAllocator<U> &other) 
    : allocator_{other.allocator_} { }

  T *allocate(std::size_t n) {
    return static_cast<T *>(allocator_->allocate(n * sizeof(T)));
  }

  void deallocate(T *p, std::size_t n) {
    allocator_->deallocate(p);
  }

  foundation::Allocator *allocator_;
};

template<typename Allocator>
struct StdDeleter
{
  StdDeleter() : allocator_{} { }

  StdDeleter(const Allocator &allocator) 
    : allocator_{allocator} { }

  using traits = std::allocator_traits<Allocator>;
  using pointer = typename traits::pointer;

  void operator()(pointer p) const {
    Allocator allocator{allocator_};
    traits::destroy(allocator, std::addressof(*p));
    traits::deallocate(allocator, p, 1);
  }
  
  Allocator allocator_;
};

template<typename T, typename U>
bool operator==(const StdAllocator<T> &lhs, const StdAllocator<U> &rhs) {
  return &lhs.allocator_ == &rhs.allocator_;
}

template<typename T, typename U>
bool operator!=(const StdAllocator<T> &lhs, const StdAllocator<U> &rhs) {
  return !(lhs == rhs);
}

} // namespace knight
