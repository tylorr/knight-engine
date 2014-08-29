set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

set(SOURCES
    common.cpp
    shader_program.cpp
    shader.cpp
    buffer_object.cpp
    vertex_array.cpp
    entity.cpp
    entity_manager.cpp
    slot_map.cpp
    system.cpp
    component.cpp
    uniform.cpp
    uniform_factory.cpp
)

add_library(knight-engine ${SOURCES})

find_package(GLEW REQUIRED)
# find_package(Lua REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/include)

include_directories(${PROJECT_SOURCE_DIR}/third-party/glfw/include)
include_directories(${GLEW_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/third-party/glm)
include_directories(${PROJECT_SOURCE_DIR}/third-party/logog/include)
# include_directories(${LUA_INCLUDE_DIR})
# include_directories(${PROJECT_SOURCE_DIR}/third-party/stb_image)


target_link_libraries(knight-engine glfw ${GLFW_LIBRARIES})
target_link_libraries(knight-engine ${GLEW_LIBRARIES})
target_link_libraries(knight-engine logog)
# target_link_libraries(demo stb_image)
# target_link_libraries(demo ${LUA_LIBRARIES})

