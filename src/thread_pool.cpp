#include "thread_pool.h"

using std::thread;

namespace knight {

void ThreadPool::Start() {
  running_ = true;

  while (pool_.size() < kThreadCount) {
    pool_.push_back(thread(&ThreadPool::Run, this));
  }
}

void ThreadPool::Add(Task task) {
  queue_.push(task);
}

void ThreadPool::Sync() const {
  // wait until queue is empty or Stop() has been triggered
  std::unique_lock<std::mutex> lk(mutex_);
  sync_condition_.wait(lk, [this]{ return queue_.size() == 0 || !running_; });
}

void ThreadPool::Stop() {
  // signal all threads to break loop
  running_ = false;

  // Add noop so that waiting threads can do a task and exit loop
  Add([]{});

  // join each thread
  for (Pool::iterator it = pool_.begin(); it != pool_.end(); ++it) {
    it->join();
  }

  pool_.clear();
}

void ThreadPool::Run() {
  while (running_) {
    Task task(queue_.wait_pop());
    task();

    sync_condition_.notify_one();
  }

  // Add noop so that waiting threads can do a task and exit loop
  // Note: After a call to Stop() queue_ will have extra noop tasks
  Add([]{});
}

} // namespace knight
