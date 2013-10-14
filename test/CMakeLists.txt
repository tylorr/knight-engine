add_executable(knight-test knight_tests.cpp)

include_directories(${PROJECT_SOURCE_DIR}/third-party/googletest/include)

target_link_libraries(knight-test gtest_main -pthread)
