#ifndef WORK_QUEUE_H_
#define WORK_QUEUE_H_

#include "common.h"

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace knight {

template<typename T>
class WorkQueue {
 public:
  WorkQueue() : should_stop_(false) { }

  void Add(T item) {
    std::lock_guard<std::mutex> lk(mutex_);
    queue_.push(item);
    condition_.notify_one();
  }

  T Remove() {
    std::unique_lock<std::mutex> lk(mutex_);
    condition_.wait(lk, [this]{
      return queue_.size() > 0 || should_stop_;
    });

    if (!should_stop_) {
      T item = queue_.front();
      queue_.pop();
      return item;
    } else {
      // TODO: find better way to handle Stopping queue
      return T();
    }
  }

  void Stop() {
    std::lock_guard<std::mutex> lk(mutex_);
    should_stop_ = true;
    condition_.notify_all();
  }

  size_t Size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
  }

 private:
  std::queue<T> queue_;
  mutable std::mutex mutex_;
  std::condition_variable condition_;

  bool should_stop_;

  KNIGHT_DISALLOW_COPY_AND_ASSIGN(WorkQueue);
};

}; // namespace knight

#endif // WORK_QUEUE_H_
