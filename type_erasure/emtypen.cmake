if(__EMTYPEN_INCLUDED)
  return()
endif()
set(__EMTYPEN_INCLUDED TRUE)

function(begin_emtypen _target)
  set(__EMPTYPEN_TARGET ${_target} PARENT_SCOPE)
  set(__EMPTYPEN_OUTPUT PARENT_SCOPE)
endfunction()

function(emtypen _archetype _output)
  if(TARGET emtypen)
    set(_emtypen_target emtypen)
  endif()

  add_custom_command(
    OUTPUT "${_output}"
    COMMAND emtypen -c ${CMAKE_CXX_FLAGS} "$<$<BOOL:$<TARGET_PROPERTY:${__EMPTYPEN_TARGET},INCLUDE_DIRECTORIES>>:-I$<JOIN:$<TARGET_PROPERTY:${__EMPTYPEN_TARGET},INCLUDE_DIRECTORIES>, -I>>" "${_archetype}" > "${CMAKE_CURRENT_BINARY_DIR}/${_output}"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS "${_archetype}" ${_emtypen_target}
    VERBATIM)

  set(__EMPTYPEN_OUTPUT ${__EMPTYPEN_OUTPUT} ${_output} PARENT_SCOPE)
endfunction()

function(end_emtypen)
  include_directories(${CMAKE_CURRENT_BINARY_DIR})
  add_custom_target(${__EMPTYPEN_TARGET}_emtypen DEPENDS ${__EMPTYPEN_OUTPUT})
  add_dependencies(${__EMPTYPEN_TARGET} ${__EMPTYPEN_TARGET}_emtypen)
endfunction()
