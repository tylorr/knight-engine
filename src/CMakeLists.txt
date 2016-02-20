set(SOURCES
    common.cpp
    buffer_object.cpp
    array_object.cpp
    entity_manager.cpp
    uniform.cpp
    material.cpp
    imgui_manager.cpp
    job_system.cpp
    stb_impl.cpp
    udp_listener.cpp
    mesh_component.cpp
    transform_component.cpp
    dependency_injection.cpp
    attribute.cpp
    buddy_allocator.cpp
    win32/windows_util.cpp
    editor/project_editor.cpp
    ../third-party/flatbuffers/src/idl_parser.cpp
    ../third-party/flatbuffers/src/idl_gen_text.cpp
    ../third-party/flatbuffers/src/reflection.cpp
)

include_directories(${KNIGHT_ENGINE_INCLUDES})

add_library(knight-engine ${SOURCES})

# register_event_fbs(knight-engine ../schema/event_header.fbs)
register_event_fbs(knight-engine ../assets/schema/types.fbs)
register_event_fbs(knight-engine ../assets/schema/transform_component.fbs)
register_event_fbs(knight-engine ../assets/schema/mesh_component.fbs)
register_event_fbs(knight-engine ../assets/schema/entity.fbs)
register_event_fbs(knight-engine ../assets/schema/entity_list.fbs)

register_event_fbs(knight-engine ../assets/schema/editor/meta.fbs)

target_link_libraries(knight-engine glfw ${GLFW_LIBRARIES})
target_link_libraries(knight-engine ${GLEW_LIBRARIES})
target_link_libraries(knight-engine logog)
target_link_libraries(knight-engine foundation)
target_link_libraries(knight-engine enet)
target_link_libraries(knight-engine imgui)
target_link_libraries(knight-engine tinyobjloader)
target_link_libraries(knight-engine dbghelp)
target_link_libraries(knight-engine ${Boost_LIBRARIES})
target_link_libraries(knight-engine crossguid)
# target_link_libraries(demo stb_image)
# target_link_libraries(demo ${LUA_LIBRARIES})

pdb(knight-engine)
cotire(knight-engine)
