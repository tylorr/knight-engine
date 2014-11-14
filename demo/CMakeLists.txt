set(SOURCES
    main.cpp
)

add_definitions(-DDEVELOPMENT)

include_directories(${KNIGHT_ENGINE_INCLUDES})

add_executable(demo ${SOURCES})
target_link_libraries(demo knight-engine)

register_event_fbs(demo schema/monster.fbs schema/event_header.fbs)

begin_emtypen(demo)
  emtypen("archetypes/any_foo.hpp" "any_foo.h")
  emtypen("archetypes/any_bar.hpp" "any_bar.h")
end_emtypen()

cotire(demo)

add_library(script SHARED script.cpp)
target_link_libraries(script knight-engine)

add_executable(client client.cpp)
target_link_libraries(client knight-engine)
