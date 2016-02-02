#pragma once

#include "pointers.h"
#include "std_allocator.h"

namespace knight {

class PointerWrapper {
 public:
  PointerWrapper(PointerWrapper &&other) {
    *this = std::move(other);
  }

  PointerWrapper &operator=(PointerWrapper &&other) {
    raw_ptr_ = other.raw_ptr_;
    other.raw_ptr_ = nullptr;
    return *this;
  }

  virtual ~PointerWrapper() = default;

  void *get() { return raw_ptr_; }

 protected:
  explicit PointerWrapper(void *raw_ptr) :
    raw_ptr_(raw_ptr) {}

 private:
  void *raw_ptr_;
};


namespace pointer_wrapper {

  namespace detail {

    template<typename T, typename Deleter>
    class UniquePointerWrapper : public PointerWrapper {
     public:
      explicit UniquePointerWrapper(std::unique_ptr<T, Deleter> &&p) :
        PointerWrapper(p.get()),
        pointer_(std::move(p)) {}

      UniquePointerWrapper(UniquePointerWrapper &&other) {
        *this = std::move(other);
      }

      auto operator=(UniquePointerWrapper &&other) {
        pointer_ = std::move(other);
        PointerWrapper::operator=(std::move(other));
        return *this;
      }

      ~UniquePointerWrapper() override = default;

     private:
      std::unique_ptr<T, Deleter> pointer_;
    };

  } // namespace detail

  template<typename T, typename Deleter>
  auto create(std::unique_ptr<T, Deleter> &&ptr) -> Pointer<PointerWrapper> {
    return std::make_unique<detail::UniquePointerWrapper<T, Deleter>>(std::move(ptr));
  }

  template<typename T>
  auto create(Pointer<T> &&ptr) -> Pointer<PointerWrapper> {
    auto &deleter = ptr.get_deleter();
    return allocate_unique<detail::UniquePointerWrapper<T, StdDeleter>>(*deleter.allocator, std::move(ptr));
  }

} // namespace pointer_wrapper

} // namespace knight
