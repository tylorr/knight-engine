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
    system.cpp
    component.cpp
    uniform.cpp
    uniform_factory.cpp
    imgui_manager.cpp
    task_manager.cpp
)

include_directories(${KNIGHT_ENGINE_INCLUDES})

add_library(knight-engine ${SOURCES})

target_link_libraries(knight-engine glfw ${GLFW_LIBRARIES})
target_link_libraries(knight-engine ${GLEW_LIBRARIES})
target_link_libraries(knight-engine logog)
target_link_libraries(knight-engine foundation)
# target_link_libraries(demo stb_image)
# target_link_libraries(demo ${LUA_LIBRARIES})

cotire(knight-engine)
