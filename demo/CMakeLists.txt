set(SOURCES
    main.cpp
    game_code.cpp
)

set(PROTO_OUTPUT "${PROJECT_BINARY_DIR}/proto")
include_directories(${PROTO_OUTPUT})
include_directories(${KNIGHT_ENGINE_INCLUDES})
include_directories("../assets")

add_executable(demo ${SOURCES})
target_link_libraries(demo knight-engine)
target_link_libraries(demo dbghelp)
pdb(demo)

#begin_emtypen(demo)
#  emtypen("archetypes/any_foo.hpp" "any_foo.h")
#  emtypen("archetypes/any_bar.hpp" "any_bar.h")
#end_emtypen()

add_library(game SHARED
    game.cpp
    ../assets/editor_component.cpp
    # TODO: Find a better way to do this

)
target_link_libraries(game knight-engine)
target_link_libraries(game dbghelp)
pdb(game)


# add_executable(client client.cpp)
# target_link_libraries(client knight-engine)

add_dependencies(demo game)

