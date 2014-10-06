#pragma once

#include "common.h"
#include "queue.h"

using knight::error;

namespace foundation {

template<typename P, typename T>
struct IndexIterator {
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef T value_type;
  typedef T &reference;
  typedef T *pointer;
  typedef IndexIterator<P, T> self_type;

  IndexIterator(P *parent, size_type index) 
    : _parent(parent), _index(index) { }

  self_type &operator++() {
    ++_index;
    return *this;
  }

  self_type operator++(int) {
    self_type old(*this);
    operator++();
    return old;
  }

  self_type &operator+=(size_type n) {
    _index += n;
    return *this;
  }

  friend self_type operator+(const self_type &it, size_type n) {
    self_type temp = it;
    return temp += n;
  }

  friend self_type operator+(size_type n, const self_type &it) {
    return it + n;
  }

  self_type &operator--() {
    --_index;
    return *this;
  }

  self_type operator--(int) {
    self_type old(*this);
    operator--();
    return old;
  }

  self_type &operator-=(size_type n) {
    return operator+=(-n);
  }

  friend self_type operator-(const self_type &it, size_type n) {
    self_type temp = it;
    return temp -= n;
  }

  friend self_type operator-(size_type n, const self_type &it) {
    return it - n;
  }

  friend difference_type operator-(self_type a, self_type b) {
    return a._index - b._index;
  }

  reference operator[](size_type n) const {
    return (*_parent)[_index + n];
  }

  reference operator*() const {
    return (*_parent)[_index];
  }

  pointer operator->() const {
    return &(*_parent)[_index];
  }

  friend bool operator==(const self_type &lhs, const self_type &rhs) {
    return lhs._parent == rhs._parent && lhs._index == rhs._index;
  }

  friend bool operator!=(const self_type &lhs, const self_type &rhs) {
    return !(lhs == rhs);
  }

  friend bool operator<(const self_type &lhs, const self_type &rhs) {
    XASSERT(lhs._parent == rhs._parent, "Comparing index iterators with different parents");
    return (rhs - lhs) > 0;
  }

  friend bool operator>(const self_type &lhs, const self_type &rhs) {
    XASSERT(lhs._parent == rhs._parent, "Comparing index iterators with different parents");
    return rhs < lhs;
  }

  friend bool operator>=(const self_type &lhs, const self_type &rhs) {
    XASSERT(lhs._parent == rhs._parent, "Comparing index iterators with different parents");
    return !(lhs < rhs);
  }

  friend bool operator<=(const self_type &lhs, const self_type &rhs) {
    XASSERT(lhs._parent == rhs._parent, "Comparing index iterators with different parents");
    return !(lhs > rhs);
  }

  P *_parent;
  size_type _index;
};

namespace index_iterator {
  template<typename P, typename T>
  IndexIterator<P, T> begin(P &parent);

  template<template<typename> class P, typename T>
  IndexIterator<P<T>, T> begin(P<T> &parent);

  template<typename T, size_t N>
  IndexIterator<T[N], T> begin(T (&parent)[N]);

  template<typename T, size_t N>
  IndexIterator<T[N], T> end(T (&parent)[N]);

  template<typename T>
  IndexIterator<Queue<T>, T> end(Queue<T> &parent);
} // namespace index_iterator

namespace index_iterator {
  template<typename P, typename T>
  IndexIterator<P, T> begin(P &parent) {
    return IndexIterator<P, T>(&parent, 0);
  }

  template<template<typename> class P, typename T>
  IndexIterator<P<T>, T> begin(P<T> &parent) {
    return begin<P<T>, T>(parent);
  }

  template<typename T, size_t N>
  IndexIterator<T[N], T> begin(T (&parent)[N]) {
    return begin<T[N], T>(parent);
  }

  template<typename T, size_t N>
  IndexIterator<T[N], T> end(T (&parent)[N]) {
    return IndexIterator<T[N], T>(&parent, N);
  }

  template<typename T>
  IndexIterator<Queue<T>, T> end(Queue<T> &parent) {
    return IndexIterator<Queue<T>, T>(&parent, queue::size(parent));
  }
} // namespace index_iterator

} // namespace foundation
