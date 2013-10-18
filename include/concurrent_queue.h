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

  void push(T item) {
    std::lock_guard<std::mutex> lk(mutex_);
    queue_.push(item);
    condition_.notify_one();
  }

  T wait_pop() {
    std::unique_lock<std::mutex> lk(mutex_);
    condition_.wait(lk, [this]{ return queue_.size() > 0; });

    T item(std::move(queue_.front()));
    queue_.pop();
    return item;
  }

  bool try_pop(T &item) {
    std::lock_guard<std::mutex> lk(mutex_);

    bool result = false;
    if (!queue_.empty()) {
      item = queue_.front();
      queue_.pop();
      result = true;
    }

    return result;
  }

  size_t size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
  }

  bool empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
  }

 private:
  std::queue<T> queue_;
  std::condition_variable condition_;

  mutable std::mutex mutex_;

  KNIGHT_DISALLOW_COPY_AND_ASSIGN(ConcurrentQueue);
};

}; // namespace knight

#endif // CONCURRENT_QUEUE_H_
