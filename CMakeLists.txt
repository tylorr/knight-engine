cmake_minimum_required(VERSION 2.8)
project(knight-engine)

enable_testing()

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

message(STATUS ${CMAKE_CXX_COMPILER})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wno-mismatched-tags -Wno-switch -fcolor-diagnostics")

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)

include(cotire)

set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_INSTALL OFF)
set(BUILD_SHARED_LIBS ON)

add_definitions(-DGLM_FORCE_RADIANS)

macro(add_fbs _fb_list_name)
  file(RELATIVE_PATH _relPath "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
  foreach(_schema ${ARGN})
    if (_relPath)
      list(APPEND ${_fb_list_name} "${_relPath}/${_schema}")
    else()
      list(APPEND ${_fb_list_name} "${_schema}")
    endif()
  endforeach()
  if (_relPath)
    set (${_fb_list_name} ${${_fb_list_name}} PARENT_SCOPE)
  endif()
endmacro()

macro(register_event_fbs _target)
  add_fbs(EVENT_FB_SCHEMAS ${ARGN})
  if(EVENT_FB_SCHEMAS)
    add_dependencies(${_target} fbschema)
    include_directories(${FBSCHEMA_OUTPUT})
  endif()
endmacro()

set(FBSCHEMA_OUTPUT "${PROJECT_BINARY_DIR}/schema_headers/")

find_package(GLEW REQUIRED)
# find_package(Lua REQUIRED)

set(KNIGHT_ENGINE_INCLUDES
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/type_erasure
  ${PROJECT_SOURCE_DIR}/third-party/glfw/include
  ${GLEW_INCLUDE_DIRS}
  ${PROJECT_SOURCE_DIR}/third-party/glm
  ${PROJECT_SOURCE_DIR}/third-party/logog/include
  ${PROJECT_SOURCE_DIR}/third-party/stb_image
  ${PROJECT_SOURCE_DIR}/third-party/imgui
  ${PROJECT_SOURCE_DIR}/third-party/flatbuffers/include
  ${PROJECT_SOURCE_DIR}/third-party/foundation
  ${PROJECT_SOURCE_DIR}/third-party/enet/include
  ${PROJECT_SOURCE_DIR}/third-party/assimp/include
  #${LUA_INCLUDE_DIR}
)

add_subdirectory(third-party/glfw)
add_subdirectory(third-party/logog)
add_subdirectory(third-party/imgui)
add_subdirectory(third-party/flatbuffers)
add_subdirectory(third-party/foundation)
add_subdirectory(third-party/enet)
add_subdirectory(third-party/assimp)
add_subdirectory(type_erasure)
add_subdirectory(src)
add_subdirectory(demo)
add_subdirectory(test)

set(FBSCHEMAS ${EVENT_FB_SCHEMAS})

if (FBSCHEMAS)
  foreach(_schema ${FBSCHEMAS})
    get_filename_component(_basename ${_schema} NAME_WE)
    list(APPEND FBSCHEMA_HEADERS "${FBSCHEMA_OUTPUT}${_basename}_generated.h")
  endforeach()

  add_custom_command(
      OUTPUT ${FBSCHEMA_HEADERS}
      COMMAND flatc -c -o "${FBSCHEMA_OUTPUT}" ${FBSCHEMAS}
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
      DEPENDS ${FBSCHEMAS} flatc)

  add_custom_target(fbschema DEPENDS ${FBSCHEMA_HEADERS})
  add_dependencies(fbschema flatc)
endif()
