#include "gtest/gtest.h"
#include "thread_pool.h"

#include <atomic>

using knight::ThreadPool;

class ThreadPoolTest : public ::testing::Test {
 protected:
  ThreadPoolTest() : pool_(3) { }

  ThreadPool pool_;
};

TEST_F(ThreadPoolTest, AddTask) {
  EXPECT_EQ(0, pool_.task_count());

  pool_.AddTask([&] { });

  EXPECT_EQ(1, pool_.task_count());

  pool_.AddTask([&] { });

  EXPECT_EQ(2, pool_.task_count());
}

TEST_F(ThreadPoolTest, StartAndWait) {
  std::atomic_uint result(0);

  int numTasks = 10;

  pool_.Start();

  for (int i = 0; i < numTasks; ++i) {
    pool_.AddTask([&]{ result++; });
  }

  pool_.WaitAll();

  EXPECT_EQ(numTasks, result.load());
}
