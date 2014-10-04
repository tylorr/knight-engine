#pragma once

#include "collection_types.h"
#include "array.h"

#include <utility>

namespace foundation {

namespace heap {
  template<typename T>
  void build(T *begin, T *end);

  template<typename T>
  bool is_valid(T *begin, T *end);

  template<typename T>
  void push(T *begin, T *end);

  template<typename T>
  void pop(T *begin, T *end);
} // namespace heap

namespace heap_internal {
  inline int32_t parent(int32_t index) {
    return (index - 1) / 2;
  }

  inline int32_t left(int32_t index) {
    return (2 * index) + 1;
  }

  inline int32_t right(int32_t index) {
    return (2 * index) + 2;
  }

  template<typename T>
  void up_heap(T *begin, T *end, int32_t index) {
    int32_t parent_index = parent(index);

    if (parent_index >= 0 && begin[parent_index] < begin[index]) {
      std::swap(begin[parent_index], begin[index]);
      up_heap(begin, end, parent_index);
    }
  }

  template<typename T>
  void down_heap(T *begin, T *end, int32_t index) {
    int32_t left_index = left(index);
    int32_t right_index = right(index);

    int32_t largest_index = index;

    int32_t heap_length = end - begin;
    if (left_index < heap_length && begin[largest_index] < begin[left_index]) {
      largest_index = left_index;
    }

    if (right_index < heap_length && begin[largest_index] < begin[right_index]) {
      largest_index = right_index;
    }

    if (largest_index != index) {
      std::swap(begin[index], begin[largest_index]);
      down_heap(begin, end, largest_index);
    }
  }
}

namespace heap {
  template<typename T>
  void build(T *begin, T *end) {
    int32_t length = end - begin;
    for (int i = (length / 2) - 1; i >= 0; --i) {
      heap_internal::down_heap(begin, end, i);
    }
  }

  template<typename T>
  bool is_valid(T *begin, T *end) {
    int32_t heap_length = end - begin;
    for (int32_t i = (heap_length / 2) - 1; i >= 0; --i) {
      int32_t left_index = heap_internal::left(i);
      int32_t right_index = heap_internal::right(i);

      if ((left_index < heap_length && begin[i] < begin[left_index]) ||
          (right_index < heap_length && begin[i] < begin[right_index])) {
        return false;
      }
    }
    return true;
  }

  template<typename T>
  void push(T *begin, T *end) {
    int32_t heap_length = end - begin;
    heap_internal::up_heap(begin, end, heap_length - 1);
  }

  template<typename T>
  void pop(T *begin, T *end) {
    int32_t heap_length = end - begin;
    T *new_end = begin + heap_length - 1;
    std::swap(begin[0], new_end[0]);

    heap_internal::down_heap(begin, new_end, 0);
  }
} // namespace heap

} // namespace foundation
