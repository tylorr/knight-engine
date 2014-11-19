#pragma once

#include "common.h"

namespace knight {

template<typename T>
class bind_guard {
 public:
  explicit bind_guard(const T &target) : target_(target) {
    target_.Bind();
  }

  ~bind_guard() {
    target_.Unbind();
  }

 private:
  const T &target_;

  KNIGHT_DISALLOW_COPY_AND_ASSIGN(bind_guard);
  KNIGHT_DISALLOW_MOVE_AND_ASSIGN(bind_guard);
};

template<typename T>
class unique_bind {
 public:
  unique_bind() : target_(nullptr) { }

  unique_bind(unique_bind &&other) 
      : target_(std::move(other.target_)) {
    other.target_ = nullptr;
  }

  explicit unique_bind(T &target)
      : target_(&target) {
    target_->Bind();
  }

  ~unique_bind() {
    if (target_) {
      target_->Unbind();
      target_ = nullptr;
    }
  }

  unique_bind &operator=(unique_bind &&other) {
    target_ = std::move(other.target_);
    other.target_ = nullptr;
  }

  void bind() {
    XASSERT(target_ != nullptr, "Cannot bind without a target");
    target_->Bind();
  }

  void unbind() {
    XASSERT(target_ != nullptr, "Cannot unbind without a target");
    target_->Unbind();
  }

  T *release() {
    T *result = target_;
    target_ = nullptr;
    return result;
  }

  T *get() {
    return target_;
  }

 private:
  T *target_;
};

} // namespace knight
