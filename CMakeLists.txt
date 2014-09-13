cmake_minimum_required(VERSION 2.8)
project(knight-engine)

enable_testing()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall") # -Wno-mismatched-tags -fcolor-diagnostics")

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)

include(cotire)

set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_INSTALL OFF)

add_definitions(-DGLM_FORCE_RADIANS)

add_subdirectory(third-party/glfw)
add_subdirectory(third-party/logog)
add_subdirectory(third-party/imgui)
add_subdirectory(src)
add_subdirectory(demo)
add_subdirectory(test)
