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
    path.cpp
    ../third-party/flatbuffers/src/idl_parser.cpp
    ../third-party/flatbuffers/src/idl_gen_text.cpp
    ../third-party/flatbuffers/src/reflection.cpp
)

set(protos
  assets/proto/object_collection.proto
  assets/proto/transform_component.proto
  assets/proto/types.proto
  assets/proto/component.proto
  assets/proto/meta.proto
)

set(PROTO_OUTPUT "${PROJECT_BINARY_DIR}/proto")

macro(compile_proto_file filename)
  get_filename_component(basename ${filename} NAME_WE)
  get_filename_component(dirname ${filename} PATH)
  add_custom_command(
    OUTPUT ${PROTO_OUTPUT}/${dirname}/${basename}.pb.cc
    DEPENDS protoc "${PROJECT_SOURCE_DIR}/${filename}"
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMAND protoc ${PROJECT_SOURCE_DIR}/${dirname}/${basename}.proto
        --proto_path=${PROJECT_SOURCE_DIR}
        --proto_path=${PROJECT_SOURCE_DIR}/third-party/protobuf/src
        --cpp_out=${PROTO_OUTPUT}
  )
endmacro(compile_proto_file)

set(proto_files)
foreach(proto_file ${protos})
  compile_proto_file(${proto_file})
  # get_filename_component(basename ${proto_file} NAME_WE)
  string(REPLACE .proto .pb.cc pb_file ${proto_file})
  set(proto_files ${proto_files}
      ${PROTO_OUTPUT}/${pb_file})
endforeach(proto_file)

include_directories(${PROTO_OUTPUT})
include_directories(${KNIGHT_ENGINE_INCLUDES})

add_library(knight-engine ${SOURCES} ${proto_files})



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
target_link_libraries(knight-engine libprotobuf)
# target_link_libraries(demo stb_image)
# target_link_libraries(demo ${LUA_LIBRARIES})

pdb(knight-engine)
cotire(knight-engine)
