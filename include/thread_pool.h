#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include "common.h"
#include "concurrent_queue.h"

#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>

namespace knight {

class ThreadPool {
 public:
  typedef std::function<void()> Task;

  const unsigned int kThreadCount;

  ThreadPool() : ThreadPool(std::thread::hardware_concurrency()) { }
  explicit ThreadPool(unsigned int count) : kThreadCount(count) { }

  ~ThreadPool() {
    Stop();
  }

  // Start all threads
  void Start();

  // Add task to job queue
  void Add(Task);

  // Wait for all tasks to be finished
  void Sync() const;

  // Join all threads
  void Stop();

  size_t task_count() const { return queue_.size(); }

 private:
  typedef std::vector<std::thread> Pool;

  void Run();

  ConcurrentQueue<Task> queue_;
  Pool pool_;

  mutable std::condition_variable sync_condition_;
  mutable std::mutex mutex_;

  std::atomic_bool running_;

  KNIGHT_DISALLOW_COPY_AND_ASSIGN(ThreadPool);
};

} // namespace knight;

#endif // THREAD_POOL_H_
