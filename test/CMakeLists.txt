set(SOURCES
    slot_map_test.cpp
    work_queue_test.cpp
    thread_pool_test.cpp
    test_main.cpp
)

add_executable(knight-test ${SOURCES})

include_directories(${PROJECT_SOURCE_DIR}/third-party/googletest/include)
include_directories(${PROJECT_SOURCE_DIR}/third-party/logog/include)
include_directories(${PROJECT_SOURCE_DIR}/include)

target_link_libraries(knight-test knight-engine gtest -pthread)
