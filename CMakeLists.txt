cmake_minimum_required(VERSION 2.8)
project(knight-engine)

enable_testing()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -Wall") # -Wno-mismatched-tags -fcolor-diagnostics")

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)

include(cotire)

set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_INSTALL OFF)

add_definitions(-DGLM_FORCE_RADIANS)

macro(add_fbs)
  file(RELATIVE_PATH _relPath "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
  foreach(_schema ${ARGN})
    if (_relPath)
      list(APPEND FBSCHEMAS "${_relPath}/${_schema}")
    else()
      list(APPEND FBSCHEMAS "${_schema}")
    endif()
  endforeach()
  if (_relPath)
    # propagate FBSCHEMAS to parent directory
    set (FBSCHEMAS ${FBSCHEMAS} PARENT_SCOPE)
  endif()
endmacro()

macro(fbs _target)
  if(FBSCHEMAS)
    add_dependencies(${_target} fbschema)
    include_directories(${FBSCHEMA_OUTPUT})
  endif()
endmacro()

set(FBSCHEMA_OUTPUT "${PROJECT_BINARY_DIR}/schema_headers/")

add_definitions(-DMSPACES)
add_definitions(-DUSE_DL_PREFIX)

add_subdirectory(third-party/glfw)
add_subdirectory(third-party/logog)
add_subdirectory(third-party/imgui)
add_subdirectory(third-party/flatbuffers)
add_subdirectory(third-party/dlmalloc)
add_subdirectory(src)
add_subdirectory(demo)
add_subdirectory(test)

if (FBSCHEMAS)
  foreach(_schema ${FBSCHEMAS})
    get_filename_component(_basename ${_schema} NAME_WE)
    list(APPEND FBSCHEMA_HEADERS "${FBSCHEMA_OUTPUT}${_basename}_generated.h")
  endforeach()

  add_custom_command(
      OUTPUT ${FBSCHEMA_HEADERS}
      COMMAND flatc -c -o "${FBSCHEMA_OUTPUT}" ${FBSCHEMAS}
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
      DEPENDS ${FBSCHEMAS})

  add_custom_target(fbschema DEPENDS ${FBSCHEMA_HEADERS})
  add_dependencies(fbschema flatc)
endif()
