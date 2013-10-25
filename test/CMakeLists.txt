set(SOURCES
    test_main.cpp
    slot_map_test.cpp
    concurrent_queue_test.cpp
    thread_pool_test.cpp
    component_manager_test.cpp
    system_test.cpp
)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")

add_executable(knight-test ${SOURCES})

include_directories(${PROJECT_SOURCE_DIR}/third-party/googletest/include)
include_directories(${PROJECT_SOURCE_DIR}/third-party/logog/include)
include_directories(${PROJECT_SOURCE_DIR}/include)

target_link_libraries(knight-test knight-engine gtest -pthread)
