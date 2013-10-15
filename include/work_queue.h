#ifndef WORK_QUEUE_H_
#define WORK_QUEUE_H_

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace knight {

template<typename T>
class WorkQueue {
 public:
  WorkQueue() { }

  void Add(T item) {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(item);
    condition_.notify_one();
  }

  T Remove() {
    std::unique_lock<std::mutex> lock(mutex_);
    condition_.wait(lock, [this]{ return queue_.size() > 0; });

    T item = queue_.front();
    queue_.pop();
    return item;
  }

  size_t Size() {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
  }

 private:
  std::queue<T> queue_;
  std::mutex mutex_;
  std::condition_variable condition_;
};

}; // namespace knight

#endif // WORK_QUEUE_H_
