set(SOURCES
    main.cpp
)

include_directories(${KNIGHT_ENGINE_INCLUDES})

add_executable(demo ${SOURCES})
target_link_libraries(demo knight-engine)

#begin_emtypen(demo)
#  emtypen("archetypes/any_foo.hpp" "any_foo.h")
#  emtypen("archetypes/any_bar.hpp" "any_bar.h")
#end_emtypen()

cotire(demo)

add_library(game SHARED game.cpp)
target_link_libraries(game knight-engine)

add_executable(client client.cpp)
target_link_libraries(client knight-engine)

add_dependencies(demo game)
