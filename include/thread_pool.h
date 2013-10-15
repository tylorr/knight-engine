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
      : kThreadCount(count),
        completed_count_(0) {}

  void Init();
  void AddTask(Task);
  void WaitAll();

 private:
  DISALLOW_COPY_AND_ASSIGN(ThreadPool);

  void Run();

  WorkQueue<Task> queue_;
  std::vector<std::thread> threads_;
  std::mutex mutex_;
  std::condition_variable join_condition_;
  std::atomic_uint completed_count_;
};

}; // namespace knight;

#endif // THREAD_POOL_H_
