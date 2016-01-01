//citation: https://github.com/Microsoft/GSL

#pragma once

#include <type_traits>

namespace knight {

namespace gsl {

template<typename T>
class owner {
 public:
  owner(T t) : ptr_{t} {}
  owner(const owner &other) = default;

  template <typename U, typename Dummy = std::enable_if_t<std::is_convertible<U, T>::value>>
  owner(const owner<U> &other) : ptr_{other.get()} {}

  owner<T> &operator=(const T &t) {
    ptr_ = t;
    return *this;
  }
    
  T get() const { return ptr_; }

  operator T() const {  return get(); }
  T operator->() const { return get(); }

  bool operator==(const T &rhs) const { return ptr_ == rhs; }
  bool operator!=(const T &rhs) const { return !(*this == rhs); }
 
 private:
  T ptr_;

  // unwanted operators...pointers only point to single objects!
  owner<T> &operator++() = delete;
  owner<T> &operator--() = delete;
  owner<T> operator++(int) = delete;
  owner<T> operator--(int) = delete;
  owner<T> &operator+(size_t) = delete;
  owner<T> &operator+=(size_t) = delete;
  owner<T> &operator-(size_t) = delete;
  owner<T> &operator-=(size_t) = delete;
};

template<typename T>
class not_null {
 public:
  not_null(T t) : ptr_{t} { ensure_invariant(); }

  // deleting these two prevents compilation when initialized with a nullptr or literal 0
  not_null(std::nullptr_t) = delete;
  not_null(int) = delete;

  not_null(const not_null &other) = default;

  template <typename U, typename Dummy = std::enable_if_t<std::is_convertible<U, T>::value>>
  not_null(const not_null<U> &other) : ptr_{other.get()} {}

  not_null<T> &operator=(const T &t) {
    ptr_ = t; 
    ensure_invariant(); 
    return *this;
  }

  // prevents compilation when someone attempts to assign a nullptr 
  not_null<T> &operator=(std::nullptr_t) = delete;
  not_null<T> &operator=(int) = delete;
    
  T get() const { return ptr_; }

  operator T() const {  return get(); }
  T operator->() const { return get(); }

  bool operator==(const T &rhs) const { return ptr_ == rhs; }
  bool operator!=(const T &rhs) const { return !(*this == rhs); }
 
 private:
  T ptr_;

  void ensure_invariant() const {
    XASSERT(ptr_ != nullptr, "nullptr passed to not_null");
  }

  // unwanted operators...pointers only point to single objects!
  not_null<T> &operator++() = delete;
  not_null<T> &operator--() = delete;
  not_null<T> operator++(int) = delete;
  not_null<T> operator--(int) = delete;
  not_null<T> &operator+(size_t) = delete;
  not_null<T> &operator+=(size_t) = delete;
  not_null<T> &operator-(size_t) = delete;
  not_null<T> &operator-=(size_t) = delete;
};

template<typename T>
struct type_trait {
  using reference = T &;
};

template<>
struct type_trait<void> {
  using reference = void;
};

template<>
struct type_trait<const void> {
  using reference = void;
};

template<typename T>
class span {
 public:
  using value_type = T;
  using reference = std::add_lvalue_reference_t<value_type>;
  using pointer = std::add_pointer_t<value_type>;

  span() : span(nullptr, 0) {}
  span(std::nullptr_t) : span(nullptr, 0) {}

  template <class IntType, typename = std::enable_if_t<std::is_integral<IntType>::value>>
  span(std::nullptr_t, IntType size) : span(nullptr, 0) {
    XASSERT(size == 0, "Cannot create nullptr span with size other than 0");
  }

  span(pointer data, std::size_t size) :
    data_{data},
    size_{size} {}

  template<typename U, std::size_t N, typename Dummy = std::enable_if_t<std::is_convertible<U*, T*>::value, void>>
  span(U (&data)[N]) :
    data_{data},
    size_{N} {}

  typename type_trait<T>::reference operator[](std::size_t index) {
    XASSERT(data_ != nullptr && index >= 0 && index < size_, "Out of bounds error");
    return data_[index];
  }
  
  const typename type_trait<T>::reference operator[](std::size_t index) const {
    XASSERT(data_ != nullptr && index >= 0 && index < size_, "Out of bounds error");
    return data_[index];
  }

  auto begin() { return data_; }
  const auto begin() const { return data_; }
  const auto cbegin() const { return data_; }

  auto end() { return data_ + size_; }
  const auto end() const { return data_ + size_; }
  const auto cend() const { return data_ + size_; }

  auto data() { return data_; }
  const auto data() const { return data_; }

  auto size() const { return size_; }

 private:
  T *data_;
  std::size_t size_;
};

template<typename CharT>
using basic_string_span = span<CharT>;

using string_span = basic_string_span<char>;
using cstring_span = basic_string_span<const char>;

} // namespace gsl

using zstring = char *;
using wzstring = wchar_t *;
using czstring = const char *;
using cwzstring = const wchar_t *;

} // namespace knight
