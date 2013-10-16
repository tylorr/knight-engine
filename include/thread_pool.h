#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include "common.h"
#include "work_queue.h"

#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace knight {

typedef std::function<void()> Task;

class ThreadPool {
 public:
  const unsigned int kThreadCount;

  ThreadPool(unsigned int count)
      : kThreadCount(count) {}

  ~ThreadPool() {
    Stop();
  }

  // Start all threads
  void Start();

  // Add task to job queue
  void AddTask(Task);

  // Wait for all tasks to be finished
  void WaitAll();

  // Join all threads
  void Stop();

  size_t task_count() const { return queue_.Size(); }

 private:
  typedef std::vector<std::thread> Pool;

  void Run();

  WorkQueue<Task> queue_;
  Pool pool_;
  std::mutex mutex_;
  std::condition_variable sync_condition_;

  std::atomic_bool running_;

  KNIGHT_DISALLOW_COPY_AND_ASSIGN(ThreadPool);
};

}; // namespace knight;

#endif // THREAD_POOL_H_
