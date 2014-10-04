set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

set(SOURCES
    main.cpp
)

add_definitions(-DDEVELOPMENT)

include_directories(${KNIGHT_ENGINE_INCLUDES})

add_executable(demo ${SOURCES})
register_event_fbs(demo schema/monster.fbs schema/event_header.fbs)

target_link_libraries(demo knight-engine)
target_link_libraries(demo imgui)
cotire(demo)
