#include "thread_pool.h"

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
  std::unique_lock<std::mutex> lk(mutex_);
  sync_condition_.wait(lk, [this]{ return queue_.Size() == 0 || running_; });
}

void ThreadPool::Stop() {
  // signal all threads to break loop
  running_ = false;

  // signal job queue to stop waiting for new tasks
  queue_.Stop();

  // join each thread
  for (Pool::iterator it = pool_.begin(); it != pool_.end(); ++it) {
    it->join();
  }

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
