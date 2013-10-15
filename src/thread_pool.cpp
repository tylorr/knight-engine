#include "thread_pool.h"

#include <logog.hpp>

using std::vector;
using std::thread;

namespace knight {

void ThreadPool::Start() {
  running_ = true;
  while (pool_.size() < kThreadCount) {
    pool_.push_back(thread(&ThreadPool::Run, this));
  }
}

void ThreadPool::AddTask(Task task) {
  queue_.Add(task);
}

void ThreadPool::WaitAll() {
  if (running_) {
    std::unique_lock<std::mutex> lk(mutex_);
    sync_condition_.wait(lk, [this]{ return queue_.Size() == 0; });
  } else {
    ALERT("Waiting on stopped thread pool, WaitAll() has been ignored");
  }
}

void ThreadPool::Stop() {
  // signal all threads to break loop
  running_ = false;

  queue_.Stop();
  // join each thread
  for (Pool::iterator it = pool_.begin(); it != pool_.end(); ++it) {
    it->join();
  }

  // destruct each thread object
  pool_.clear();
}

void ThreadPool::Run() {
  while (running_) {
    // printf("Waiting...\n");
    Task task = queue_.Remove();

    if (task) {
      task();
    }
    sync_condition_.notify_one();
  }
}

}; // namespace knight
