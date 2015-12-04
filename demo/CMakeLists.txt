set(SOURCES
    main.cpp
    game_code.cpp
)

include_directories(${KNIGHT_ENGINE_INCLUDES})

add_executable(demo ${SOURCES})
target_link_libraries(demo knight-engine)
target_link_libraries(demo dbghelp)
pdb(demo)

#begin_emtypen(demo)
#  emtypen("archetypes/any_foo.hpp" "any_foo.h")
#  emtypen("archetypes/any_bar.hpp" "any_bar.h")
#end_emtypen()

cotire(demo)

add_library(game SHARED
    game.cpp
    # TODO: Find a better way to do this
    ../third-party/flatbuffers/src/idl_parser.cpp
    ../third-party/flatbuffers/src/idl_gen_text.cpp
    ../third-party/flatbuffers/src/reflection.cpp
)
target_link_libraries(game knight-engine)
target_link_libraries(game dbghelp)
pdb(game)


# add_executable(client client.cpp)
# target_link_libraries(client knight-engine)

add_dependencies(demo game)

