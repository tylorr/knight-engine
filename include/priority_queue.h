#pragma once

#include "collection_types.h"
#include "array.h"
#include "queue.h"
#include "heap.h"
#include "index_iterator.h"

#include <logog.hpp>

namespace foundation
{
  template <typename T>
  struct PriorityQueue {
    PriorityQueue(Allocator &a);
    PriorityQueue(const Queue<T> &v);
    PriorityQueue(Allocator &a, const Array<T> &v);

    Queue<T> _data;
  };

  namespace priority_queue 
  {
    /// Returns the number of items in the queue.
    template <typename T> uint32_t size(const PriorityQueue<T> &q);

    template <typename T> bool empty(const PriorityQueue<T> &q);

    /// Inserts a new element in the priority queue
    template<typename T> void push(PriorityQueue<T> &q, const T &item);

    /// Removes the element on top of the priority queue
    template<typename T> void pop(PriorityQueue<T> &q);

    /// Retrieve the element on top of the priority queue
    template<typename T> const T &top(const PriorityQueue<T> &q);

    template<typename T, typename... Args> void emplace(PriorityQueue<T> &q, Args&&... args);
  }

  namespace priority_queue 
  {
    template<typename T>
    inline uint32_t size(const PriorityQueue<T> &q) {
      return queue::size(q._data);
    }

    template<typename T>
    inline bool empty(const PriorityQueue<T> &q) {
      return queue::size(q._data) == 0;
    }

    template<typename T>
    void push(PriorityQueue<T> &q, const T &item) {
      queue::push_back(q._data, item);
      heap::push(index_iterator::begin(q._data), index_iterator::end(q._data));
    }

    template<typename T>
    void pop(PriorityQueue<T> &q) {
      heap::pop(index_iterator::begin(q._data), index_iterator::end(q._data));
      queue::pop_back(q._data);
    }

    template<typename T>
    const T &top(const PriorityQueue<T> &q) {
      return q._data[0];
    }

    template<typename T, typename... Args>
    void emplace(PriorityQueue<T> &q, Args&&... args) {
      queue::emplace(q._data, std::forward<Args>(args)...);
      heap::push(index_iterator::begin(q._data), index_iterator::end(q._data));
    }
  }

  template <typename T> 
  inline PriorityQueue<T>::PriorityQueue(Allocator &allocator) 
      : _data(allocator) { }

  template<typename T>
  inline PriorityQueue<T>::PriorityQueue(const Queue<T> &queue)
      : _data(queue) { 
    heap::build(index_iterator::begin(_data), index_iterator::end(_data));
  }

  template<typename T>
  inline PriorityQueue<T>::PriorityQueue(Allocator &allocator, const Array<T> &array)
      : _data(allocator) {
    queue::push(_data, array::begin(array), array::size(array));    
    heap::build(index_iterator::begin(_data), index_iterator::end(_data));
  }
}
