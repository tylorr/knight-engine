set(SOURCES
    entity_manager_test.cpp
)

add_executable(knight-test ${SOURCES})

include_directories(${PROJECT_SOURCE_DIR}/third-party/googletest/include)
include_directories(${PROJECT_SOURCE_DIR}/include)

target_link_libraries(knight-test knight-engine gtest_main -pthread)
