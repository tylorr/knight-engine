#include "task_manager.h"

#include "common.h"
#include "slot_map.h"

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

  SlotMap<Task, Task::ID> open_tasks_;
  std::priority_queue<Task *, std::vector<Task *>, TaskPointerComparator> work_queue_;
  std::unordered_map<Task::ID, Task::ID> dependency_map_;

  std::vector<std::thread> thread_pool_;
};

TaskManagerData *task_manager_data_;
foundation::Allocator *allocator_;

void TryCloseTask(const Task::ID &task_id);
void TryCloseTaskRecursively(Task *task);

void DoWork() {
  Task *task;
  while (true) {
    {
      std::unique_lock<std::mutex> lock(task_manager_data_->mutex_);
      task_manager_data_->condition_.wait(lock, [] {
        return !task_manager_data_->work_queue_.empty() || task_manager_data_->stop_; 
      });

      if (task_manager_data_->stop_) {
        return;
      }

      task = task_manager_data_->work_queue_.top();
      task_manager_data_->work_queue_.pop();
    }

    task->work_item_.work_function(task->work_item_);

    TryCloseTask(task->id_);
  }
}

void TryCloseTask(const Task::ID &task_id) {
  {
    std::unique_lock<std::mutex> lock(task_manager_data_->mutex_);

    auto *task = task_manager_data_->open_tasks_.Get(task_id);
    TryCloseTaskRecursively(task);
  }

  task_manager_data_->condition_.notify_one();
}

void TryCloseTaskRecursively(Task *task) {
  XASSERT(task != nullptr, "Trying to close non-existent task");

  if (--task->open_work_items_ == 0) {
    auto *parent_task = task_manager_data_->open_tasks_.Get(task->parent_id_);

    if (parent_task != nullptr) {
      TryCloseTaskRecursively(parent_task);
    }

    auto dependent_it = task_manager_data_->dependency_map_.find(task->id_);

    if (dependent_it != task_manager_data_->dependency_map_.end()) {
      auto *dependent_task = task_manager_data_->open_tasks_.Get(dependent_it->second);

      task_manager_data_->work_queue_.emplace(dependent_task);
      task_manager_data_->dependency_map_.erase(dependent_it);
    }

    task_manager_data_->open_tasks_.Destroy(task->id_);
  }
}

Task *CreateTask(const WorkItem &work_item) {
  auto task_id = task_manager_data_->open_tasks_.Create();
  auto *task = task_manager_data_->open_tasks_.Get(task_id);
  task->work_item_ = work_item;
  return task;
}

} // namespace

void Start(foundation::Allocator &allocator, const unsigned int &thread_count) {
  XASSERT(task_manager_data_ == nullptr, "Trying to call start more than once");

  allocator_ = &allocator;
  task_manager_data_ = allocator_->make_new<TaskManagerData>(allocator);

  task_manager_data_->stop_ = false;

  for (unsigned int i = 0; i < thread_count; ++i) {
    task_manager_data_->thread_pool_.emplace_back(DoWork);
  }
}

void Stop() {
  task_manager_data_->stop_ = true;

  task_manager_data_->condition_.notify_all();

  for (size_t i = 0; i < task_manager_data_->thread_pool_.size(); ++i) {
    task_manager_data_->thread_pool_[i].join();
  }

  task_manager_data_->thread_pool_.clear();

  allocator_->make_delete(task_manager_data_);
}

Task::ID BeginAdd(const WorkItem &work_item) {
  Task *task;
  {
    std::lock_guard<std::mutex> lock(task_manager_data_->mutex_);

    task = CreateTask(work_item);

    // Add dummy open work item so that task cannot complete until FinishAdd()
    // is called
    task->open_work_items_++;
    
    task_manager_data_->work_queue_.emplace(task);
  }

  task_manager_data_->condition_.notify_one();

  return task->id_;
}

Task::ID BeginAddWithDependency(const WorkItem &work_item, 
                                const Task::ID &dependency_id) {
  std::lock_guard<std::mutex> lock(task_manager_data_->mutex_);

  XASSERT(task_manager_data_->open_tasks_.Get(dependency_id) != nullptr, "Trying to add a non-existent task as a dependency: %lu", dependency_id);

  auto *task = CreateTask(work_item);

  // Add dummy open work item so that task cannot complete until FinishAdd()
  // is called
  task->open_work_items_++;

  task->dependency_id_ = dependency_id;

  task_manager_data_->dependency_map_.emplace(dependency_id, task->id_);

  return task->id_;
}

Task::ID BeginAddEmpty() {
  return BeginAdd(kEmptyWorkItem);
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
    std::lock_guard<std::mutex> lock(task_manager_data_->mutex_);

    Task *task;
    for (size_t i = 0; i < count; ++i) {
      task = CreateTask(work_items[i]);
      task_manager_data_->work_queue_.emplace(task);
    }
  }

  task_manager_data_->condition_.notify_one();
}

void AddChild(const Task::ID &parent_id, const Task::ID &child_id) {
  std::lock_guard<std::mutex> lock(task_manager_data_->mutex_);

  auto *parent_task = task_manager_data_->open_tasks_.Get(parent_id);

  XASSERT(parent_task != nullptr, "Trying to add a child to non-existent task: %lu <- %lu", parent_id, child_id);

  auto *child_task = task_manager_data_->open_tasks_.Get(child_id);

  XASSERT(child_task != nullptr, "Trying to add a non-existent task as a child: %lu <- %lu", parent_id, child_id);

  parent_task->open_work_items_++;
  child_task->parent_id_ = parent_id;
}

void Wait(const Task::ID &task_id_to_wait_for) {
  Task *task;
  while (true) {
    {
      std::unique_lock<std::mutex> lock(task_manager_data_->mutex_);
      task_manager_data_->condition_.wait(lock, [&task_id_to_wait_for] {
        return !task_manager_data_->work_queue_.empty() ||
               task_manager_data_->open_tasks_.Get(task_id_to_wait_for) == nullptr ||
               task_manager_data_->stop_; 
      });

      if (task_manager_data_->open_tasks_.Get(task_id_to_wait_for) == nullptr || task_manager_data_->stop_) {
        return;
      }

      task = task_manager_data_->work_queue_.top();
      task_manager_data_->work_queue_.pop();
    }

    task->work_item_.work_function(task->work_item_);

    TryCloseTask(task->id_);
  }
}

} // namespace TaskManager

} // namespace knight
