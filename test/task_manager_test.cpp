// #include "common.h"
// #include "task_manager.h"

// #include <catch.hpp>

// #include <vector>

// using namespace knight;
// using std::vector;

// void DoWork(const WorkItem &work_item) {
//   auto *completed = (vector<int> *)work_item.arg1.ptr;
//   completed->push_back(work_item.arg2.i);
// }

// TEST_CASE("Task Manager") {
//   TaskManager::Start(1);

//   vector<int> completed_tasks;

//   WorkItem work_item(DoWork);

//   work_item.arg1.ptr = (void *)&completed_tasks;

//   // SECTION("Tasks complete work") {
//   //   work_item.arg2.i = 10;
//   //   auto task_id = TaskManager::BeginAdd(work_item);
//   //   TaskManager::FinishAdd(task_id);

//   //   TaskManager::Wait(task_id);

//   //   CHECK(completed_tasks[0] == 10);
//   // }

//   SECTION("A task is not finished until all it's children are finished") {
//     //auto task_id = TaskManager::BeginAddEmpty();

//     // work_item.arg2.i = 0;
//     auto child1_id = TaskManager::BeginAdd(work_item);
//     auto child2_id = TaskManager::BeginAdd(work_item);

//     // TaskManager::AddChild(task_id, child1_id);
//     // TaskManager::AddChild(task_id, child2_id);

//     TaskManager::FinishAdd(child1_id);
//     TaskManager::FinishAdd(child2_id);

//     //TaskManager::FinishAdd(task_id);

//     // TaskManager::Wait(task_id);

//     // CHECK(completed_tasks.size() == 2u);
//   }

//   // work_item.arg1.i = 1;
//   // work_item.arg2.i = 700;
//   // auto task_id = TaskManager::BeginAdd(work_item);

//   // work_item.arg1.i = 10;
//   // work_item.arg2.i = 800;
//   // auto child_id = TaskManager::BeginAdd(work_item);

//   // TaskManager::AddChild(task_id, child_id);

//   // TaskManager::FinishAdd(child_id);

//   // work_item.arg1.i = 1;
//   // work_item.arg2.i = 500;
//   // auto d_task_id = TaskManager::BeginAddWithDependency(work_item, task_id);

//   // TaskManager::FinishAdd(d_task_id);
//   // TaskManager::FinishAdd(task_id);

//   // work_item.arg1.i = 2;
//   // work_item.arg2.i = 0;
//   // TaskManager::Add(1, &work_item);

//   // TaskManager::Wait(d_task_id);

//   TaskManager::Stop();
// }
