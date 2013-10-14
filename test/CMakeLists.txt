add_executable(knight-test knight_tests.cpp)

include_directories(${PROJECT_SOURCE_DIR}/third-party/googletest/include)
include_directories(${PROJECT_SOURCE_DIR}/include)

target_link_libraries(knight-test knight-engine gtest_main -pthread)
