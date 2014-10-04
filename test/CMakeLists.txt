set(SOURCES
    test_main.cpp
    slot_map_test.cpp
    # thread_pool_test.cpp
    entity_manager_test.cpp
    # entity_bundle_test.cpp
    task_manager_test.cpp
    heap_test.cpp
)

add_definitions(-DLOGOG_USE_PREFIX)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/third-party/catch)
include_directories(${PROJECT_SOURCE_DIR}/third-party/stb_image)
include_directories(${PROJECT_SOURCE_DIR}/third-party/logog/include)
include_directories(${PROJECT_SOURCE_DIR}/third-party/foundation)

add_executable(knight_test ${SOURCES})
target_link_libraries(knight_test knight-engine)
cotire(knight_test)

add_test(NAME knight_test_all COMMAND knight_test)
