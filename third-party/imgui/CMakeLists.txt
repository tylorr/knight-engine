project(imgui)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-function")

add_library(imgui imgui.cpp)
