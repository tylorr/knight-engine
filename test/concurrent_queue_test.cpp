#include "gtest/gtest.h"
#include "concurrent_queue.h"

#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

using knight::ConcurrentQueue;
using std::thread;
using std::chrono::milliseconds;

typedef std::function<void()> Task;

class ConcurrentQueueTest : public ::testing::Test {
 protected:
  ConcurrentQueue<Task> queue_;
};

TEST_F(ConcurrentQueueTest, AddAndSize) {
  queue_.push([] { printf("Task 1"); });

  EXPECT_EQ(1, queue_.size());

  queue_.push([] { printf("Task 2"); });

  EXPECT_EQ(2, queue_.size());
}

void RemoveThread(ConcurrentQueue<Task> &q) {
  Task t;
  q.wait_pop(t);
  t();
}

TEST_F(ConcurrentQueueTest, WaitAfterAdd) {
  int result = 0;

  queue_.push([&] { result = 1; });

  std::thread t(RemoveThread, std::ref(queue_));

  t.join();

  EXPECT_EQ(1, result);
}

TEST_F(ConcurrentQueueTest, WaitBeforeAdd) {
  int result = 0;

  std::thread t(RemoveThread, std::ref(queue_));

  queue_.push([&] { result = 1; });

  t.join();

  EXPECT_EQ(1, result);
}

void MissedAddThread(ConcurrentQueue<Task> &q) {
  Task t;
  q.wait_pop(t);
  t();

  Task t2;
  q.wait_pop(t2);
  t2();
}

TEST_F(ConcurrentQueueTest, MissedAddSignal) {
  int result = 0;

  std::thread t(MissedAddThread, std::ref(queue_));

  queue_.push([&] {
    milliseconds longDuration(100);
    std::this_thread::sleep_for(longDuration);
    result = 1;
  });

  // Add() triggers condition_variable on thread, fortunately it is waiting on
  // variable with predicate: queue.size() > 1 so that it doesn't miss new tasks
  queue_.push([&] {
    result = 2;
  });

  t.join();

  EXPECT_EQ(2, result);
}

