set(SOURCES
    common.cpp
    buffer_object.cpp
    mesh.cpp
    entity_manager.cpp
    uniform.cpp
    material.cpp
    imgui_manager.cpp
    job_system.cpp
    stb_impl.cpp
    udp_listener.cpp
    string_util.cpp
    mesh_component.cpp
    transform_component.cpp
    game_memory.cpp
    dependency_injection.cpp
    attribute.cpp
    win32/file_read.cpp
    win32/file_write.cpp
    win32/file_util.cpp
    win32/windows_utils.cpp
)

include_directories(${KNIGHT_ENGINE_INCLUDES})

add_library(knight-engine ${SOURCES})

# register_event_fbs(knight-engine ../schema/event_header.fbs)
register_event_fbs(knight-engine ../schema/types.fbs)
register_event_fbs(knight-engine ../schema/transform_component.fbs)
register_event_fbs(knight-engine ../schema/mesh_component.fbs)
register_event_fbs(knight-engine ../schema/entity_resource.fbs)

target_link_libraries(knight-engine glfw ${GLFW_LIBRARIES})
target_link_libraries(knight-engine ${GLEW_LIBRARIES})
target_link_libraries(knight-engine logog)
target_link_libraries(knight-engine foundation)
target_link_libraries(knight-engine enet)
target_link_libraries(knight-engine imgui)
target_link_libraries(knight-engine tinyobjloader)
target_link_libraries(knight-engine dbghelp)
# target_link_libraries(demo stb_image)
# target_link_libraries(demo ${LUA_LIBRARIES})

pdb(knight-engine)
cotire(knight-engine)
