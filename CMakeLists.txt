cmake_minimum_required(VERSION 2.8)
project(knight-engine)

enable_testing()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY D:/projects/knight-engine/build/clion)

message(STATUS ${CMAKE_CXX_COMPILER})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -pthread -Wall -Wno-comment -Wno-switch")

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)

set (COTIRE_UNITY_OUTPUT_DIRECTORY "")
include(cotire)

set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "" FORCE)
set(GLFW_INSTALL OFF CACHE INTERNAL "" FORCE)
set(protobuf_BUILD_TESTS OFF CACHE INTERNAL "" FORCE)

set(BUILD_SHARED_LIBS ON)

add_definitions(-DGLM_FORCE_RADIANS)
add_definitions(-DDEVELOPMENT)
add_definitions(-DIMGUI_INCLUDE_IMCONFIG_H)
add_definitions(-DUNICODE=1)
add_definitions(-D_UNICODE=1)
add_definitions(-DBOOST_FILESYSTEM_NO_DEPRECATED=1)
add_definitions(-DBOOST_SYSTEM_NO_DEPRECATED=1)

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
  endif()
endmacro()

macro(pdb target)
  add_custom_command(TARGET ${target} POST_BUILD COMMAND ${PROJECT_SOURCE_DIR}/bin/cv2pdb $<TARGET_FILE:${target}>)
endmacro(pdb)

set(FBSCHEMA_OUTPUT "${PROJECT_BINARY_DIR}/schema_headers/")

find_package(GLEW REQUIRED)
find_package(Boost 1.60 REQUIRED COMPONENTS filesystem system)

set(KNIGHT_ENGINE_INCLUDES
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/include/win32 # TODO: TR Choose by platform
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
  ${PROJECT_SOURCE_DIR}/third-party/utf8
  ${PROJECT_SOURCE_DIR}/third-party/tinyobjloader
  ${PROJECT_SOURCE_DIR}/third-party/pcg/include
  ${PROJECT_SOURCE_DIR}/third-party/gsl/include
  ${PROJECT_SOURCE_DIR}/third-party/crossguid
  ${PROJECT_SOURCE_DIR}/third-party/protobuf/src
  ${PROJECT_SOURCE_DIR}/third-party/cereal/include
  ${FBSCHEMA_OUTPUT}
  ${Boost_INCLUDE_DIR}
)

add_subdirectory(third-party/glfw)
add_subdirectory(third-party/logog)
add_subdirectory(third-party/imgui)

set(FLATBUFFERS_FLATC_EXECUTABLE flatc)
add_subdirectory(third-party/flatbuffers)
add_subdirectory(third-party/foundation)
add_subdirectory(third-party/enet)
add_subdirectory(third-party/tinyobjloader)
add_subdirectory(third-party/gsl)
add_subdirectory(third-party/crossguid)
add_subdirectory(third-party/protobuf/cmake)
add_subdirectory(type_erasure)
add_subdirectory(src)
add_subdirectory(demo)
add_subdirectory(test)

set(FBSCHEMAS ${EVENT_FB_SCHEMAS})

if (FBSCHEMAS)
  set(SCHEMA_BINARY_HEADER "${FBSCHEMA_OUTPUT}schema_binaries.h")
  file(WRITE ${SCHEMA_BINARY_HEADER} "//Generated headers include\n#pragma once\n")
  foreach(_schema ${FBSCHEMAS})
    get_filename_component(_basename ${_schema} NAME_WE)
    list(APPEND FBSCHEMA_HEADERS "${FBSCHEMA_OUTPUT}${_basename}_generated.h")
    list(APPEND FBSCHEMA_BINARIES "${FBSCHEMA_OUTPUT}${_basename}.bfbs")
    list(APPEND FBSCHEMA_BINARY_HEADERS "${FBSCHEMA_OUTPUT}${_basename}_schema.h")

    add_custom_command(
      OUTPUT "${FBSCHEMA_OUTPUT}${_basename}_schema.h"
      COMMAND xxd -i "${_basename}.bfbs" "${_basename}_schema.h"
      WORKING_DIRECTORY ${FBSCHEMA_OUTPUT}
      DEPENDS "${FBSCHEMA_OUTPUT}${_basename}.bfbs")

    file(APPEND ${SCHEMA_BINARY_HEADER} "#include \"${_basename}_schema.h\"\n")
  endforeach()

  add_custom_command(
      OUTPUT ${FBSCHEMA_HEADERS}
      COMMAND flatc -c --gen-mutable -o "${FBSCHEMA_OUTPUT}" ${FBSCHEMAS}
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
      DEPENDS ${FBSCHEMAS} flatc)

  add_custom_command(
      OUTPUT ${FBSCHEMA_BINARIES}
      COMMAND flatc -b --schema -o "${FBSCHEMA_OUTPUT}" ${FBSCHEMAS}
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
      DEPENDS ${FBSCHEMAS} flatc)

  add_custom_target(fbschema DEPENDS ${FBSCHEMA_HEADERS} ${FBSCHEMA_BINARIES} ${FBSCHEMA_BINARY_HEADERS})
  add_dependencies(fbschema flatc)
endif()


