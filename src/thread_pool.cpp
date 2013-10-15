#include "thread_pool.h"

namespace knight {

void ThreadPool::Init() {
  for (int i = 0; i < kThreadCount; ++i) {
    threads_.push_back(std::thread(&ThreadPool::Run, this));
  }
}

void ThreadPool::AddTask(Task task) {
  queue_.Add(task);
}

void ThreadPool::WaitAll() {
  std::unique_lock<std::mutex> lock(mutex_);
  join_condition_.wait(lock, [this]{ return completed_count_ == kThreadCount; });
  completed_count_ = 0;
}

void ThreadPool::Run() {
  while (true) {
    Task task = queue_.Remove();
    task();
    completed_count_++;
    join_condition_.notify_one();
  }
}

}; // namespace knight
