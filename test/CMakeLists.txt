set(SOURCES
    test_main.cpp
    slot_map_test.cpp
    entity_manager_test.cpp
    index_iterator_test.cpp
    # task_manager_test.cpp
    heap_test.cpp
    priority_queue_test.cpp
    transform_component_test.cpp
)

add_definitions(-DLOGOG_USE_PREFIX)

include_directories(${KNIGHT_ENGINE_INCLUDES})
include_directories(${PROJECT_SOURCE_DIR}/third-party/catch)

add_executable(knight_test ${SOURCES})
target_link_libraries(knight_test knight-engine)

add_test(NAME knight_test_all COMMAND knight_test)
