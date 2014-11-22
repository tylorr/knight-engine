#pragma once

#include "common.h"

#include <memory.h>

#include <limits>
#include <thread>

namespace knight {

struct WorkItem {
  union DataBlock { int i; float vec[4]; void *ptr; };

  WorkItem() : work_function(nullptr) { }
  WorkItem(void (*work_function)(const WorkItem &)) : work_function(work_function) { }

  DataBlock arg1;
  DataBlock arg2;
  DataBlock arg3;
  DataBlock arg4;

  void (*work_function)(const WorkItem &);
};

struct Task {
  using ID = ID64<Task>::ID;

  Task::ID id;
  WorkItem work_item;
  Task::ID parent_id;
  Task::ID dependency_id;
  int open_work_items;
  int priority;

  Task() : id(0),
           parent_id(0),
           dependency_id(0),
           open_work_items(1),
           priority(0) { }

  int operator<(const Task &other) const {
    return priority > other.priority;
  }
};

namespace TaskManager {

void Start(foundation::Allocator &allocator, const unsigned int &thread_count = std::thread::hardware_concurrency());
void Stop();

Task::ID BeginAdd(const WorkItem &work_item, const Task::ID &dependency_id = 0);
Task::ID BeginAddEmpty();

void FinishAdd(const Task::ID &task_id);

void Add(const size_t &count, const WorkItem *work_items);

void AddChild(const Task::ID &parent_id, const Task::ID &child_id);

void Wait(const Task::ID &task_id_to_wait_for);

} // namespace TaskManager

} // namespace knight

namespace std {

// std::hash specialization for Entity::ID
template<>
struct hash<knight::Task::ID> {
  size_t operator()(const knight::Task::ID &id) const {
    return hash<uint64_t>()(id.id);
  }
};

} // namespace std
