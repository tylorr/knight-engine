project(imgui)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-function")

set(SOURCES
    imgui.cpp
    imgui_draw.cpp
    imgui_demo.cpp
)

add_library(imgui ${SOURCES})
target_link_libraries(imgui imm32)
