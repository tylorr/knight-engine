#pragma once

#include <memory.h>

#include <cstddef>

namespace knight {

template<typename T>
struct StdAllocator {
 public:
  using value_type = T;

  template<typename U>
  struct rebind { using other = StdAllocator<U>; };

  StdAllocator(foundation::Allocator &allocator) 
    : allocator_{allocator} { }

  template<typename U>
  StdAllocator(const StdAllocator<U> &other) 
    : allocator_{other.allocator_} { }

  T *allocate(std::size_t n) {
    return static_cast<T *>(allocator_.allocate(n * sizeof(T)));
  }

  void deallocate(T *p, std::size_t n) {
    allocator_.deallocate(p);
  }

  foundation::Allocator &allocator_;
};

template<typename Alloc>
struct StdDeleter
{
  StdDeleter(const Alloc &alloc) : alloc_{alloc} { }

  using pointer = typename std::allocator_traits<Alloc>::pointer;

  void operator()(pointer p) const {
    Alloc alloc{alloc_};
    std::allocator_traits<Alloc>::destroy(alloc, std::addressof(*p));
    std::allocator_traits<Alloc>::deallocate(alloc, p, 1);
  }

private:
  Alloc alloc_;
};

template<typename T, typename U>
bool operator==(const StdAllocator<T> &lhs, const StdAllocator<U> &rhs) {
  return &lhs.allocator_ == &rhs.allocator_;
}

template<typename T, typename U>
bool operator!=(const StdAllocator<T> &lhs, const StdAllocator<U> &rhs) {
  return !(lhs == rhs);
}

template<typename T, typename... Args>
std::shared_ptr<T> allocate_shared(foundation::Allocator &alloc, Args&&... args) {
  return std::allocate_shared<T>(StdAllocator<T>{alloc}, std::forward<Args>(args)...);
}

template<typename T, typename... Args>
auto allocate_unique(foundation::Allocator &alloc, Args&&... args) {
  using Alloc = StdAllocator<T>;
  using AT = std::allocator_traits<Alloc>;
  static_assert(std::is_same<typename AT::value_type, std::remove_cv_t<T>>{}(),
                "Allocator has the wrong value_type");

  Alloc a{alloc};
  auto p = AT::allocate(a, 1);
  AT::construct(a, std::addressof(*p), std::forward<Args>(args)...);
  using D = StdDeleter<Alloc>;
  return std::unique_ptr<T, D>(p, D(a));
}

} // namespace knight
