set(SOURCES
    test_main.cpp
    slot_map_test.cpp
    # thread_pool_test.cpp
    entity_manager_test.cpp
    # entity_bundle_test.cpp
)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/third-party/catch)
include_directories(${PROJECT_SOURCE_DIR}/third-party/stb_image)

add_executable(knight-test ${SOURCES})
target_link_libraries(knight-test knight-engine)
cotire(knight-test)

add_test(NAME knight-test-all COMMAND knight-test)
