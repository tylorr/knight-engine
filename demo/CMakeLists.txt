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

function(begin_emtypen _target)
  set(__EMPTYPEN_TARGET ${_target} PARENT_SCOPE)
  set(__EMPTYPEN_OUTPUT PARENT_SCOPE)
endfunction()

function(emtypen _archetype _output)
  # get_property(include_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
  add_custom_command(
    OUTPUT "${_output}"
    COMMAND emtypen -c ${CMAKE_CXX_FLAGS} "$<$<BOOL:$<TARGET_PROPERTY:${__EMPTYPEN_TARGET},INCLUDE_DIRECTORIES>>:-I$<JOIN:$<TARGET_PROPERTY:${__EMPTYPEN_TARGET},INCLUDE_DIRECTORIES>, -I>>" "${_archetype}" > "${CMAKE_CURRENT_BINARY_DIR}/${_output}"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS "${_archetype}" emtypen
    VERBATIM)

  set(__EMPTYPEN_OUTPUT ${__EMPTYPEN_OUTPUT} ${_output} PARENT_SCOPE)
endfunction()

function(end_emtypen)
  include_directories(${CMAKE_CURRENT_BINARY_DIR})
  add_custom_target(${__EMPTYPEN_TARGET}_emtypen DEPENDS ${__EMPTYPEN_OUTPUT})
  add_dependencies(${__EMPTYPEN_TARGET} ${__EMPTYPEN_TARGET}_emtypen)
endfunction()

register_event_fbs(demo schema/monster.fbs schema/event_header.fbs)

begin_emtypen(demo)
  emtypen("archetypes/AnyFoo.hpp" "AnyFoo.h")
  emtypen("archetypes/AnyBar.hpp" "AnyBar.h")
end_emtypen()

cotire(demo)
