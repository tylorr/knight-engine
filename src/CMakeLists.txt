set(SOURCES
    common.cpp
    shader_program.cpp
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
    stb_impl.cpp
    udp_listener.cpp
)

include_directories(${KNIGHT_ENGINE_INCLUDES})

add_library(knight-engine ${SOURCES})

register_event_fbs(knight-engine ../schema/monster.fbs ../schema/event_header.fbs)

target_link_libraries(knight-engine glfw ${GLFW_LIBRARIES})
target_link_libraries(knight-engine ${GLEW_LIBRARIES})
target_link_libraries(knight-engine logog)
target_link_libraries(knight-engine foundation)
target_link_libraries(knight-engine enet)
target_link_libraries(knight-engine assimp)
target_link_libraries(knight-engine imgui)
# target_link_libraries(demo stb_image)
# target_link_libraries(demo ${LUA_LIBRARIES})

cotire(knight-engine)
