#include "task_manager.h"

#include "common.h"
#include "slot_map.h"
#include "semaphore.h"

#include <logog.hpp>
#include <memory.h>

#include <queue>
#include <unordered_map>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace knight {

namespace TaskManager {

namespace {

struct TaskPointerComparator {
  bool operator()(const Task *task1, const Task *task2) {
    return *task1 < *task2;
  }
};

const WorkItem kEmptyWorkItem([](const WorkItem &work_item) { });

struct TaskManagerData {
  TaskManagerData(foundation::Allocator &allocator) : open_tasks_(allocator) { }

  std::atomic_bool stop_;

  std::mutex mutex_;
  std::condition_variable condition_;
  std::condition_variable wait_condition_;

  SlotMap<Task, Task::ID> open_tasks_;
  std::priority_queue<Task *, std::vector<Task *>, TaskPointerComparator> work_queue_;
  std::unordered_map<Task::ID, Task::ID> dependency_map_;

  std::vector<std::thread> thread_pool_;
};

TaskManagerData *data_;
foundation::Allocator *allocator_;

void TryCloseTask(const Task::ID &task_id);
void TryCloseTaskRecursively(Task &task);

void DoWork(Semaphore &ready_semaphore) {
  ready_semaphore.Notify();

  Task *task;
  while (true) {
    {
      std::unique_lock<std::mutex> lock(data_->mutex_);
      data_->condition_.wait(lock, [] {
        return !data_->work_queue_.empty() || data_->stop_; 
      });

      if (data_->stop_) {
        return;
      }

      task = data_->work_queue_.top();
      data_->work_queue_.pop();
    }

    task->work_item.work_function(task->work_item);

    TryCloseTask(task->id);
  }
}

void TryCloseTask(const Task::ID &task_id) {
  {
    std::unique_lock<std::mutex> lock(data_->mutex_);

    auto *task = data_->open_tasks_.Get(task_id);
    TryCloseTaskRecursively(*task);
  }

  data_->wait_condition_.notify_all();
  data_->condition_.notify_one();
}

void TryCloseTaskRecursively(Task &task) {
  if (--task.open_work_items == 0) {
    auto *parent_task = data_->open_tasks_.Get(task.parent_id);

    if (parent_task != nullptr) {
      TryCloseTaskRecursively(*parent_task);
    }

    auto dependent_it = data_->dependency_map_.find(task.id);

    if (dependent_it != data_->dependency_map_.end()) {
      auto *dependent_task = data_->open_tasks_.Get(dependent_it->second);

      data_->work_queue_.emplace(dependent_task);
      data_->dependency_map_.erase(dependent_it);
    }

    data_->open_tasks_.Destroy(task.id);
  }
}

Task *CreateTask(const WorkItem &work_item) {
  auto task_id = data_->open_tasks_.Create();
  auto *task = data_->open_tasks_.Get(task_id);
  task->work_item = work_item;
  return task;
}

} // namespace

void Start(foundation::Allocator &allocator, const unsigned int &thread_count) {
  XASSERT(data_ == nullptr, "Trying to call start more than once");

  allocator_ = &allocator;
  data_ = allocator_->make_new<TaskManagerData>(allocator);

  data_->stop_ = false;

  // Start the semaphore with a deficit so that Wait() only triggers when
  // the last thread calls Notify()
  Semaphore ready_semaphore(1 - thread_count);

  for (unsigned int i = 0; i < thread_count; ++i) {
    data_->thread_pool_.emplace_back(DoWork, std::ref(ready_semaphore));
  }

  ready_semaphore.Wait();
}

void Stop() {
  data_->stop_ = true;

  data_->condition_.notify_all();

  for (size_t i = 0; i < data_->thread_pool_.size(); ++i) {
    data_->thread_pool_[i].join();
  }

  data_->thread_pool_.clear();

  allocator_->make_delete(data_);
  data_ = nullptr;
}

Task::ID BeginAddEmpty() {
  return BeginAdd(kEmptyWorkItem);
}

Task::ID BeginAdd(const WorkItem &work_item, const Task::ID &dependency_id) {
  Task *task;
  {
    std::lock_guard<std::mutex> lock(data_->mutex_);

    task = CreateTask(work_item);

    // Add dummy open work item so that it cannot complete until FinishAdd()
    task->open_work_items++;

    if (dependency_id.id != 0) {
      XASSERT(data_->open_tasks_.Get(dependency_id) != nullptr, 
              "Trying to add a non-existent task as a dependency: %lu", 
              dependency_id);

      task->dependency_id = dependency_id;
      data_->dependency_map_.emplace(dependency_id, task->id);
    } else {
      data_->work_queue_.emplace(task);
    }
  }

  if (dependency_id.id == 0) {
    data_->condition_.notify_one();
  }

  return task->id;
}

void FinishAdd(const Task::ID &task_id) {

  // This will clear dummy work item we added earlier
  TryCloseTask(task_id);
}

void Add(const size_t &count, const WorkItem *work_items) {
  if (count <= 0) {
    return;
  }

  {
    std::lock_guard<std::mutex> lock(data_->mutex_);

    Task *task;
    for (size_t i = 0; i < count; ++i) {
      task = CreateTask(work_items[i]);
      data_->work_queue_.emplace(task);
    }
  }

  data_->condition_.notify_one();
}

void AddChild(const Task::ID &parent_id, const Task::ID &child_id) {
  std::lock_guard<std::mutex> lock(data_->mutex_);

  auto *parent_task = data_->open_tasks_.Get(parent_id);

  XASSERT(parent_task != nullptr, "Trying to add a child to non-existent task: %lu <- %lu", parent_id, child_id);

  auto *child_task = data_->open_tasks_.Get(child_id);

  XASSERT(child_task != nullptr, "Trying to add a non-existent task as a child: %lu <- %lu", parent_id, child_id);

  parent_task->open_work_items++;
  child_task->parent_id = parent_id;
}

void Wait(const Task::ID &task_id_to_wait_for) {
  Task *task;
  while (true) {
    {
      std::unique_lock<std::mutex> lock(data_->mutex_);
      data_->wait_condition_.wait(lock, [&task_id_to_wait_for] {
        return data_->open_tasks_.Get(task_id_to_wait_for) == nullptr ||
               !data_->work_queue_.empty() ||
               data_->stop_; 
      });

      if (data_->open_tasks_.Get(task_id_to_wait_for) == nullptr || data_->stop_) {
        return;
      }

      task = data_->work_queue_.top();
      data_->work_queue_.pop();
    }

    task->work_item.work_function(task->work_item);

    TryCloseTask(task->id);
  }
}

} // namespace TaskManager

} // namespace knight
