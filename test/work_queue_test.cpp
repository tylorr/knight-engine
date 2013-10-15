#include "gtest/gtest.h"
#include "work_queue.h"

#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

using knight::WorkQueue;
using std::thread;
using std::chrono::milliseconds;

typedef std::function<void()> Task;

class WorkQueueTest : public ::testing::Test {
 protected:
  WorkQueue<Task> queue_;
};

TEST_F(WorkQueueTest, AddAndSize) {
  queue_.Add([] { printf("Task 1"); });

  EXPECT_EQ(1, queue_.Size());

  queue_.Add([] { printf("Task 2"); });

  EXPECT_EQ(2, queue_.Size());
}

void RemoveThread(WorkQueue<Task> &q) {
  Task t = q.Remove();
  t();
}

TEST_F(WorkQueueTest, RemoveAfterAdd) {
  int result = 0;

  queue_.Add([&] { result = 1; });

  std::thread t(RemoveThread, std::ref(queue_));

  t.join();

  EXPECT_EQ(1, result);
}

TEST_F(WorkQueueTest, RemoveBeforeAdd) {
  int result = 0;

  std::thread t(RemoveThread, std::ref(queue_));

  queue_.Add([&] { result = 1; });

  t.join();

  EXPECT_EQ(1, result);
}

void MissedAddThread(WorkQueue<Task> &q) {
  Task t = q.Remove();
  t();
  Task t2 = q.Remove();
  t2();
}

TEST_F(WorkQueueTest, MissedAddSignal) {
  int result = 0;

  std::thread t(MissedAddThread, std::ref(queue_));

  queue_.Add([&] {
    milliseconds longDuration(100);
    std::this_thread::sleep_for(longDuration);
    result = 1;
  });

  // Add() triggers condition_variable on thread, fortunately it is waiting on
  // variable with predicate: queue.size() > 1 so that it doesn't miss new tasks
  queue_.Add([&] {
    result = 2;
  });

  t.join();

  EXPECT_EQ(2, result);
}

TEST_F(WorkQueueTest, Stop) {
  std::thread t([this] {
    Task t = queue_.Remove();
  });

  queue_.Stop();

  t.join();
}

