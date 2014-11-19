set(SOURCES
    main.cpp
)

include_directories(${KNIGHT_ENGINE_INCLUDES})

add_executable(demo ${SOURCES})
target_link_libraries(demo knight-engine)

begin_emtypen(demo)
  emtypen("archetypes/any_foo.hpp" "any_foo.h")
  emtypen("archetypes/any_bar.hpp" "any_bar.h")
end_emtypen()

cotire(demo)

add_library(script SHARED script.cpp)
target_link_libraries(script knight-engine)

add_executable(client client.cpp)
target_link_libraries(client knight-engine)
