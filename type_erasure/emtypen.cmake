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
    set(_emtypen_path "$<TARGET_FILE:emtypen>")
  else()

    # TODO: Possibly you some sort of cmake find* method to get this
    set(_emtypen_path "emtypen")
  endif()

  set(_target_includes)
  list(APPEND _target_includes "$<TARGET_PROPERTY:${__EMPTYPEN_TARGET},INCLUDE_DIRECTORIES>")

  # TODO: Need to find a cross platform way of getting at these include directories
  set(_target_includes "$<$<BOOL:${_target_includes}>:-I$<JOIN:${_target_includes}, -I>> -IC:/msys64/mingw32/i686-w64-mingw32/include -IC:/msys64/mingw32/include/c++/4.9.1 -IC:/msys64/mingw32/include/c++/4.9.1/i686-w64-mingw32")

  # TODO: Need to find a cross platform way of generating scripts (not batch only)
  set(_command "@echo off\n${_emtypen_path} -c ${CMAKE_CXX_FLAGS} ")
  set(_emtypen_args " ${_archetype}")

  get_filename_component(_archetype_name "${_archetype}" NAME_WE)

  if(EXECUTABLE_OUTPUT_PATH)
    set(binary_path ${EXECUTABLE_OUTPUT_PATH})
  else()
    set(binary_path ${CMAKE_CURRENT_BINARY_DIR})
  endif()

  # TODO: Right now limited to batch script only
  set(_script_name "${binary_path}/${__EMPTYPEN_TARGET}_${_archetype_name}_emtypen_script.bat")

  file(GENERATE
    OUTPUT ${_script_name}
    CONTENT ${_command}${_target_includes}${_emtypen_args}
    CONDITION 1)

  add_custom_command(
    OUTPUT "${_output}"
    COMMAND ${_script_name} > ${CMAKE_CURRENT_BINARY_DIR}/${_output}
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
