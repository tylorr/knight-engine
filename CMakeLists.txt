cmake_minimum_required(VERSION 2.8)

# set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
# set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fcolor-diagnostics -std=c++11")


project(knight-engine)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)

add_subdirectory(third-party/glfw)
add_subdirectory(third-party/stb_image)
add_subdirectory(third-party/logog)
add_subdirectory(src)
add_subdirectory(demo)
