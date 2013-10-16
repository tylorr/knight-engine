cmake_minimum_required(VERSION 2.8)
project(knight-engine)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-mismatched-tags -fcolor-diagnostics -std=c++11")

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)

add_subdirectory(third-party/glfw)
add_subdirectory(third-party/stb_image)
add_subdirectory(third-party/logog)
add_subdirectory(third-party/googletest)
add_subdirectory(src)
add_subdirectory(demo)
add_subdirectory(test)
