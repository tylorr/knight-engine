#include "task_manager.h"

#include "common.h"
#include "slot_map.h"
#include "semaphore.h"
#include "priority_queue.h"

#include <logog.hpp>
#include <memory.h>
#include <array.h>
#include <hash.h>

#include <unordered_map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

using namespace foundation;

namespace knight {
namespace TaskManager {

namespace {

  class TaskPtrComparator {
   public:
    TaskPtrComparator() : task_(nullptr) { }
    TaskPtrComparator(Task *task) : task_(task) { }

    operator Task *() const { return task_; }

    int operator<(const TaskPtrComparator &other) const {
      return *task_ < *other.task_;
    }

   private:
    Task *task_;
  };

  const WorkItem kEmptyWorkItem{[](const WorkItem &work_item){}};

  struct TaskManagerData {
    TaskManagerData(Allocator &allocator) 
      : open_tasks_(allocator),
        work_queue_(allocator),
        dependency_map_(allocator),
        thread_pool_(allocator) { }

    std::atomic_bool stop_;

    std::mutex mutex_;
    std::condition_variable condition_;
    std::condition_variable wait_condition_;

    SlotMap<Task, Task::ID> open_tasks_;
    PriorityQueue<TaskPtrComparator> work_queue_;
    Hash<Task::ID> dependency_map_;
    Array<std::thread> thread_pool_;
  };

  TaskManagerData *data_;
  Allocator *allocator_;

  void TryCloseTask(const Task::ID &task_id);
  void TryCloseTaskRecursively(Task &task);

  void DoWork(Semaphore &ready_semaphore) {
    ready_semaphore.Notify();

    Task *task;
    while (true) {
      {
        std::unique_lock<std::mutex> lock(data_->mutex_);
        data_->condition_.wait(lock, [] {
          return !priority_queue::empty(data_->work_queue_) || data_->stop_; 
        });

        if (data_->stop_) {
          return;
        }

        task = priority_queue::top(data_->work_queue_);
        priority_queue::pop(data_->work_queue_);
      }

      task->work_item.work_function(task->work_item);

      TryCloseTask(task->id);
    }
  }

  void TryCloseTask(const Task::ID &task_id) {
    {
      std::lock_guard<std::mutex> lock(data_->mutex_);

      auto *task = data_->open_tasks_.Get(task_id);
      XASSERT(task != nullptr, "Trying to close non-existent task");
      
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

      if (hash::has(data_->dependency_map_, task.id)) {
        auto dependent_id = hash::get(data_->dependency_map_, task.id, Task::ID{0});
        auto *dependent_task = data_->open_tasks_.Get(dependent_id);

        priority_queue::emplace(data_->work_queue_, dependent_task);
        hash::remove(data_->dependency_map_, task.id);
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

void Start(Allocator &allocator, const unsigned int &thread_count) {
  XASSERT(data_ == nullptr, "Trying to call start more than once");

  allocator_ = &allocator;
  data_ = allocator_->make_new<TaskManagerData>(allocator);

  data_->stop_ = false;

  // Start the semaphore with a deficit so that Wait() only triggers when
  // the last thread calls Notify()
  Semaphore ready_semaphore(1 - thread_count);

  for (unsigned int i = 0; i < thread_count; ++i) {
    array::emplace_back(data_->thread_pool_, DoWork, std::ref(ready_semaphore));
  }

  ready_semaphore.Wait();
}

void Stop() {
  data_->stop_ = true;

  data_->condition_.notify_all();

  auto thread_count = array::size(data_->thread_pool_);
  for (auto i = 0u; i < thread_count; ++i) {
    data_->thread_pool_[i].join();
  }

  array::clear(data_->thread_pool_);

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
      hash::set(data_->dependency_map_, dependency_id, task->id);
    } else {
      priority_queue::emplace(data_->work_queue_, task);
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
      priority_queue::emplace(data_->work_queue_, task);
    }
  }

  data_->condition_.notify_one();
}

void AddChild(const Task::ID &parent_id, const Task::ID &child_id) {
  std::lock_guard<std::mutex> lock(data_->mutex_);

  auto *parent_task = data_->open_tasks_.Get(parent_id);

  XASSERT(parent_task != nullptr, 
          "Trying to add a child to non-existent task: %lu <- %lu", 
          parent_id, child_id);

  auto *child_task = data_->open_tasks_.Get(child_id);

  XASSERT(child_task != nullptr, 
          "Trying to add a non-existent task as a child: %lu <- %lu", 
          parent_id, child_id);

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
               !priority_queue::empty(data_->work_queue_) ||
               data_->stop_; 
      });

      if (data_->open_tasks_.Get(task_id_to_wait_for) == nullptr || data_->stop_) {
        return;
      }

      task = priority_queue::top(data_->work_queue_);
      priority_queue::pop(data_->work_queue_);
    }

    task->work_item.work_function(task->work_item);

    TryCloseTask(task->id);
  }
}

} // namespace TaskManager
} // namespace knight
