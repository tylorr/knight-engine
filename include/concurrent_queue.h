#pragma once

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
  ~ConcurrentQueue() { condition_.notify_all(); }

  void push(const T &item);
  void push(T &&item);

  void wait_pop(T &item);
  bool try_pop(T &item);

  size_t size() const;
  bool empty() const;

 private:
  std::queue<T> queue_;
  std::condition_variable condition_;

  mutable std::mutex mutex_;

  KNIGHT_DISALLOW_COPY_AND_ASSIGN(ConcurrentQueue);
};

// Template implementations

template<typename T>
void ConcurrentQueue<T>::push(const T &item) {
  {
    std::lock_guard<std::mutex> lk(mutex_);
    queue_.push(item);
  }

  condition_.notify_one();
}

template<typename T>
void ConcurrentQueue<T>::push(T &&item) {
  {
    std::lock_guard<std::mutex> lk(mutex_);
    queue_.push(std::move(item));
  }

  condition_.notify_one();
}

template<typename T>
void ConcurrentQueue<T>::wait_pop(T &item) {
  std::unique_lock<std::mutex> lk(mutex_);
  condition_.wait(lk, [this]{ return queue_.size() > 0; });

  item = queue_.front();
  queue_.pop();
}

template<typename T>
bool ConcurrentQueue<T>::try_pop(T &item) {
  std::lock_guard<std::mutex> lk(mutex_);

  if (queue_.empty()) {
    return false;
  }
  
  item = queue_.front();
  queue_.pop();

  return true;
}

template<typename T>
size_t ConcurrentQueue<T>::size() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return queue_.size();
}

template<typename T>
bool ConcurrentQueue<T>::empty() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return queue_.empty();
}

} // namespace knight
