#include "gtest/gtest.h"
#include "thread_pool.h"

#include <atomic>
#include <chrono>

using knight::ThreadPool;

class ThreadPoolTest : public ::testing::Test {
  
};

// TEST_F(ThreadPoolTest, Add) {
//   EXPECT_EQ(0, pool_.task_count());

//   pool_.Add([&] { });

//   EXPECT_EQ(1, pool_.task_count());

//   pool_.Add([&] { });

//   EXPECT_EQ(2, pool_.task_count());
// }

TEST_F(ThreadPoolTest, StartAndWait) {
  std::atomic_uint result(0);
  int numTasks = 10;
  
  ThreadPool pool(2);

  for (int i = 0; i < numTasks; ++i) {
    pool.Enqueue([&] {
      std::this_thread::sleep_for(std::chrono::milliseconds(7));
      result++; 
    });
  }

  pool.Sync();

  EXPECT_EQ(numTasks, result);
}
