#define LOGOG_LEVEL LOGOG_LEVEL_ALL

#include "gtest/gtest.h"

#include <logog.hpp>

int main(int argc, char **argv) {
  int result;

  LOGOG_INITIALIZE();
  {
    logog::Cout out;

    ::testing::InitGoogleTest(&argc, argv);
    result = RUN_ALL_TESTS();
  }
  LOGOG_SHUTDOWN();

  return result;
}
