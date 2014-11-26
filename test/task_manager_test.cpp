#include "common.h"
#include "task_manager.h"

#include <catch.hpp>

#include <vector>
#include <chrono>
#include <thread>
#include <atomic>

using namespace knight;
using namespace foundation;
using std::vector;

void DoWork(const WorkItem &work_item) {
  auto *completed = (vector<int> *)work_item.arg1.ptr;
  completed->push_back(work_item.arg2.i);

  std::chrono::milliseconds dura(work_item.arg3.i);
  std::this_thread::sleep_for(dura);
}

void DependeeWork(const WorkItem &work_item) {
  std::chrono::milliseconds dura(100);
  std::this_thread::sleep_for(dura);

  auto *completed = (vector<int> *)work_item.arg1.ptr;
  CHECK(completed->size() == 0);
}

void DependentWork(const WorkItem &work_item) {
  auto *completed = (vector<int> *)work_item.arg1.ptr;
  completed->push_back(10);
}

TEST_CASE("Task Manager") {
  TaskManager::Start(memory_globals::default_allocator());

  vector<int> completed_tasks;

  WorkItem work_item(DoWork);
  work_item.arg3.i = 100;
  work_item.arg1.ptr = (void *)&completed_tasks;

  SECTION("Tasks complete work") {
    work_item.arg2.i = 10;
    auto task_id = TaskManager::BeginAdd(work_item);
    TaskManager::FinishAdd(task_id);

    TaskManager::Wait(task_id);

    CHECK(completed_tasks[0] == 10);
  }

  SECTION("A task is not finished until all its children are finished") {
    auto parent_task_id = TaskManager::BeginAddEmpty();

      auto child_id_1 = TaskManager::BeginAdd(work_item);
      TaskManager::FinishAdd(child_id_1);
      TaskManager::AddChild(parent_task_id, child_id_1);

      auto child_id_2 = TaskManager::BeginAdd(work_item);
      TaskManager::FinishAdd(child_id_2);
      TaskManager::AddChild(parent_task_id, child_id_2);

    TaskManager::FinishAdd(parent_task_id);

    TaskManager::Wait(parent_task_id);

    CHECK(completed_tasks.size() == 2u);
  }

  SECTION("A task with dependency does not start until dependee has finished") {
    WorkItem dependee_work(DependeeWork);
    dependee_work.arg1.ptr = (void *)&completed_tasks;

    auto dependee_task = TaskManager::BeginAdd(dependee_work);
    TaskManager::FinishAdd(dependee_task);

    WorkItem dependent_work(DependentWork);
    dependent_work.arg1.ptr = (void *)&completed_tasks;
    
    auto dependent_task = TaskManager::BeginAdd(dependent_work, dependee_task);
    TaskManager::FinishAdd(dependent_task);

    TaskManager::Wait(dependent_task);
  }

  TaskManager::Stop();
}
