#ifndef KNIGHT_THREAD_POOL_H_
#define KNIGHT_THREAD_POOL_H_

#include "common.h"

#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>
#include <queue>

namespace knight {

class ThreadPool {
 public:
  ThreadPool() : ThreadPool(std::thread::hardware_concurrency()) { }
  explicit ThreadPool(const unsigned int &thread_count);

  ~ThreadPool();

  template<typename F, typename... Args>
  void Enqueue(F&& f, Args&&... args);

  void Sync() const;

 private:
  typedef std::function<void()> Task;

  std::queue<Task> queue_;
  std::vector<std::thread> thread_pool_;

  mutable std::condition_variable queue_condition_;
  mutable std::condition_variable sync_condition_;
  mutable std::mutex mutex_;

  std::atomic_bool stop_;
  std::atomic_uint unfinished_task_count_;

  KNIGHT_DISALLOW_COPY_AND_ASSIGN(ThreadPool);
  KNIGHT_DISALLOW_MOVE_AND_ASSIGN(ThreadPool);
};

ThreadPool::ThreadPool(const unsigned int &thread_count)
  : stop_(false), unfinished_task_count_(0) {
  for (unsigned int i = 0; i < thread_count; ++i) {
    thread_pool_.emplace_back([this] {
      Task task;
      while (true) {
        {
          std::unique_lock<std::mutex> lock(mutex_);
          queue_condition_.wait(lock, [this] {
            return !queue_.empty() || stop_; 
          });

          if (stop_) {
            return;
          }

          task = queue_.front();
          queue_.pop();
        }

        task();

        if (--unfinished_task_count_ == 0) {
          sync_condition_.notify_one();
        }
      }
    });
  }
}

ThreadPool::~ThreadPool() {
  stop_ = true;

  queue_condition_.notify_all();

  for (size_t i = 0; i < thread_pool_.size(); ++i) {
    thread_pool_[i].join();
  }
}

template<typename F, typename... Args>
void ThreadPool::Enqueue(F&& task_function, Args&&... args) {
  if (stop_) {
    return;
  }

  unfinished_task_count_++;

  {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(std::bind(std::forward<F>(task_function), std::forward<Args>(args)...));
  }

  queue_condition_.notify_one();
}

void ThreadPool::Sync() const {
  std::unique_lock<std::mutex> lock(mutex_);

  sync_condition_.wait(lock, [this] {
    return unfinished_task_count_ == 0 || stop_; 
  });
}

} // namespace knight;

#endif // KNIGHT_THREAD_POOL_H_
