#ifndef CONCURRENT_QUEUE_H_
#define CONCURRENT_QUEUE_H_

#include "common.h"

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace knight {

template<typename T>
class ConcurrentQueue {
 public:
  ConcurrentQueue() { }

  void push(T item);

  T wait_pop();
  bool try_pop(T &item);

  size_t size() const;
  bool empty() const;

 private:
  std::queue<T> queue_;
  std::condition_variable condition_;

  mutable std::mutex mutex_;

  KNIGHT_DISALLOW_COPY_AND_ASSIGN(ConcurrentQueue);
};

} // namespace knight

#include "concurrent_queue.tpp"

#endif // CONCURRENT_QUEUE_H_
