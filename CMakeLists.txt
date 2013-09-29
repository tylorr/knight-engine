cmake_minimum_required(VERSION 2.8)

project(game-engine)

set(CMAKE_C_COMPILER "/usr/local/bin/clang")
set(CMAKE_CXX_COMPILER "/usr/local/bin/clang++")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/MyCMakeModules)

add_subdirectory(third-party/glfw)
add_subdirectory(third-party/stb_image)
add_subdirectory(src)
