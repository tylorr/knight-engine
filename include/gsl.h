//citation: https://github.com/Microsoft/GSL

#pragma once

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
    XASSERT(ptr_ != nullptr);
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
class array_view {
 public:
  template<std::size_t N>
  array_view(T (&array)[N]) :
    array_{array},
    size_{N} {}

  array_view(T *array, std::size_t size) :
    array_{array},
    size_{size} {}

  T &operator[](std::size_t index) {
    XASSERT(array_ != nullptr && index >= 0 && index < size_, "Out of bounds error");
    return array_[index];
  }

  const T &operator[](std::size_t index) const {
    XASSERT(array_ != nullptr && index >= 0 && index < size_, "Out of bounds error");
    return array_[index];
  }

  //TODO: Implement methods you need, begin, end, etc.

 private:
  T *array_;
  std::size_t size_;
};

template<typename CharT>
using basic_string_view = array_view<CharT>;

using string_view = basic_string_view<char>;
using cstring_view = basic_string_view<const char>;

using zstring = char *;
using czstring = const char *;


} // namespace gsl

} // namespace knight
