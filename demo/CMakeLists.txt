set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

set(SOURCES
    main.cpp
)

add_definitions(-DDEVELOPMENT)

include_directories(${KNIGHT_ENGINE_INCLUDES})

add_executable(demo ${SOURCES})
target_link_libraries(demo knight-engine)
target_link_libraries(demo imgui)

register_event_fbs(demo schema/monster.fbs schema/event_header.fbs)

begin_emtypen(demo)
  emtypen("archetypes/AnyFoo.hpp" "AnyFooSpec.h")
  emtypen("archetypes/AnyBar.hpp" "AnyBarSpec.h")
end_emtypen()

cotire(demo)
