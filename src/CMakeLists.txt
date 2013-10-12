set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fcolor-diagnostics -std=c++11")

set(SOURCES
    main.cpp
    program.cpp
    shader.cpp
    buffer_object.cpp
    vertex_array.cpp
)

add_executable(game-engine ${SOURCES})

find_package(GLEW REQUIRED)
find_package(Lua REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/third-party/glfw/include)
include_directories(${GLEW_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/third-party/glm)
include_directories(${LUA_INCLUDE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/third-party/stb_image)
include_directories(${PROJECT_SOURCE_DIR}/third-party/logog/include)
include_directories(${PROJECT_SOURCE_DIR}/include)

target_link_libraries(game-engine glfw ${GLFW_LIBRARIES})
target_link_libraries(game-engine ${GLEW_LIBRARIES})
target_link_libraries(game-engine stb_image)
target_link_libraries(game-engine logog)
target_link_libraries(game-engine ${LUA_LIBRARIES})
