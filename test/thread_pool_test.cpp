#include "gtest/gtest.h"
#include "thread_pool.h"

#include <atomic>
#include <chrono>

using knight::ThreadPool;

using std::function;
using std::chrono::milliseconds;

class ThreadPoolTest : public ::testing::Test {
 protected:
  ThreadPool pool_;
};

TEST_F(ThreadPoolTest, Add) {
  EXPECT_EQ(0, pool_.task_count());

  pool_.Add([&] { });

  EXPECT_EQ(1, pool_.task_count());

  pool_.Add([&] { });

  EXPECT_EQ(2, pool_.task_count());
}

TEST_F(ThreadPoolTest, StartAndWait) {
  std::atomic_uint result(0);

  int numTasks = 10;

  pool_.Start();

  for (int i = 0; i < numTasks; ++i) {
    pool_.Add([&]{ result++; });
  }

  pool_.Sync();

  EXPECT_EQ(numTasks, result.load());
}

TEST_F(ThreadPoolTest, StopWithWaitingThreads) {
  pool_.Start();

  // Wait for threads to processing
  std::this_thread::sleep_for(std::chrono::milliseconds(200));

  pool_.Stop();

  SUCCEED();
}

TEST_F(ThreadPoolTest, StopWithLongTasks) {
  int result = 0;

  pool_.Start();

  pool_.Add([&] {
    // simulate expensive operation
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    result = 1;
  });

  // Wait for threads to processing
  std::this_thread::sleep_for(std::chrono::milliseconds(200));

  pool_.Stop();

  // succeed by getting here without stalling
  SUCCEED();

  EXPECT_EQ(1, result);
}
