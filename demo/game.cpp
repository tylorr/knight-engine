#include "game.h"
#include "game_platform.h"
#include "common.h"
#include "shader_types.h"
#include "uniform.h"
#include "imgui_manager.h"
#include "mesh_component.h"
#include "transform_component.h"
#include "entity_manager.h"
#include "types.h"
#include "pointers.h"
#include "material.h"
#include "flatbuffer_allocator.h"
#include "buffer_object.h"
#include "array_object.h"
#include "dependency_injection.h"
#include "job_system.h"
#include "file_util.h"
#include "buddy_allocator.h"
#include "vector.h"
#include "editor/project_editor.h"

#include "assets/proto/object_collection.pb.h"

// #include "entity_generated.h"
// // #include "entity_resource_generated.h"
// #include "transform_component_generated.h"
// #include "mesh_component_generated.h"
// #include "types_generated.h"
// #include "schema_binaries.h"


// #include <flatbuffers/idl.h>
// #include <flatbuffers/reflection.h>

#include <logog.hpp>
#include <string_stream.h>

#include <imgui.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/ext.hpp>

#include <tiny_obj_loader.h>

#include <cstdio>
#include <string>

using namespace knight;
using namespace foundation;
using namespace gsl;
using namespace string_stream;
using namespace google::protobuf;

using std::shared_ptr;
using std::unique_ptr;

namespace knight {
namespace detail {

  template<>
  struct attribute_traits<glm::vec3> : float_traits, component_traits<3> {};

  template<>
  struct attribute_traits<glm::vec4> : float_traits, component_traits<4> {};

} // namespace detail

// namespace schema {
//   auto glm_cast(const vec3 &vec) {
//     return glm::vec3{vec.x(), vec.y(), vec.z()};
//   }

//   auto glm_cast(const vec4 &vec) {
//     return glm::vec4{vec.x(), vec.y(), vec.z(), vec.w()};
//   }

//   auto glm_cast(const mat4 &mat) {
//     return glm::mat4{
//       glm_cast(mat.col0()),
//       glm_cast(mat.col1()),
//       glm_cast(mat.col2()),
//       glm_cast(mat.col3()),
//     };
//   }
// } // namespace schema

} // namespace knight

#include <google/protobuf/util/json_util.h>
#include <google/protobuf/util/type_resolver.h>
#include <google/protobuf/util/type_resolver_util.h>

struct Vertex {
  TrivialVec3 position;
  TrivialVec3 normal;
};

auto EntityManagerFactory() {
  auto &allocator = memory_globals::default_allocator();
  return allocate_unique<EntityManager>(allocator, allocator);
}

auto MaterialManagerFactory() {
  auto &allocator = memory_globals::default_allocator();
  return allocate_unique<MaterialManager>(allocator, allocator);
}

auto MeshComponentFactory() {
  auto &allocator = memory_globals::default_allocator();
  return allocate_unique<MeshComponent>(allocator, allocator);
}

auto TransformComponentFactory() {
  auto &allocator = memory_globals::default_allocator();
  return allocate_unique<TransformComponent>(allocator, allocator);
}

void BuildInjector(GameState &game_state) {
  auto &allocator = memory_globals::default_allocator();
  di::InjectorConfig config;

  config.add(EntityManagerFactory);
  config.add(MaterialManagerFactory);
  config.add(MeshComponentFactory);
  config.add(TransformComponentFactory);

  game_state.injector = allocate_unique<di::Injector>(allocator, config.build_injector(allocator));
}

// std::unordered_map<czstring<>, const uint8_t *> componet_map {
//   { "TransformComponent", transform_component_bfbs }
// };

GameState game_state;
std::vector<uint8_t> scene_entity_buffer;

Pointer<editor::ProjectEditor> project_editor;

static const char *kTypeUrlPrefix = "type.googleapis.com";

static std::string get_type_url(const Descriptor* message) {
  return std::string(kTypeUrlPrefix) + "/" + message->full_name();
}

int get_file_id() {
  static int current_id = 0;
  return current_id++;
}

void set_proto_vec(proto::vec3 &proto_vec, const glm::vec3 &vec) {
  proto_vec.set_x(vec.x);
  proto_vec.set_y(vec.y);
  proto_vec.set_z(vec.z);
}

extern "C" void Init(GLFWwindow &window) {
  JobSystem::initialize();

  auto &allocator = memory_globals::default_allocator();
  auto &scratch_allocator = memory_globals::default_scratch_allocator();

  // TODO: Load through injector
  project_editor = allocate_unique<editor::ProjectEditor>(allocator, allocator, "../assets");

  BuildInjector(game_state);

  auto material_manager = game_state.injector->get_instance<MaterialManager>();

  game_state.window = &window;
  ImGuiManager::initialize(window, *material_manager);

  game_state.material = material_manager->create_material("../assets/shaders/blinn_phong.shader");

  auto material = game_state.material;

  game_state.mvp_uniform = material->get<float, 4, 4>("MVP");
  game_state.mv_matrix_uniform = material->get<float, 4, 4>("ModelView");
  game_state.normal_matrix_uniform = material->get<float, 3, 3>("NormalMatrix");

  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string err = tinyobj::LoadObj(shapes, materials, "../assets/models/bench.obj", "../assets/models/");

  XASSERT(err.empty(), "Error loading model: %s", err.c_str());

  auto &obj_mesh = shapes[0].mesh;

  Vector<Vertex> vertices{scratch_allocator};
  for (auto i = 0u; i < obj_mesh.positions.size(); i += 3) {
    vertices.push_back(
      Vertex {
        { obj_mesh.positions[i], obj_mesh.positions[i+1], obj_mesh.positions[i+2] },
        { obj_mesh.normals[i], obj_mesh.normals[i+1], obj_mesh.normals[i+2] }
      });
  }

  game_state.vbo = allocate_unique<BufferObject>(allocator, BufferObject::Target::Array);
  game_state.ibo = allocate_unique<BufferObject>(allocator, BufferObject::Target::ElementArray);
  game_state.vao = allocate_unique<ArrayObject>(allocator);

  auto &vbo = *game_state.vbo;
  auto &ibo = *game_state.ibo;
  auto &vao = *game_state.vao;

  vbo.set_data(vertices, BufferObject::Usage::StaticDraw);
  ibo.set_data(gsl::as_span(obj_mesh.indices), BufferObject::Usage::StaticDraw);

  vao.set_count(obj_mesh.indices.size())
     .set_primitive(ArrayObject::Primitive::Triangles)
     .set_index_buffer(ibo, 0, ArrayObject::IndexType::UnsignedInt)
     .add_vertex_buffer(vbo, 0, Attribute<glm::vec3>{0}, Attribute<glm::vec3>{1});

  auto entity_manager = game_state.injector->get_instance<EntityManager>();
  auto entity_id = entity_manager->create();
  auto entity = entity_manager->get(entity_id);

  game_state.entity_id = entity_id;

  auto mesh_component = game_state.injector->get_instance<MeshComponent>();
  mesh_component->add(*entity, *game_state.material, *game_state.vao);

  auto transform_component = game_state.injector->get_instance<TransformComponent>();
  transform_component->add(*entity);

  auto entity_file_id = get_file_id();
  auto transform_file_id = get_file_id();
  auto child_file_id = get_file_id();

  proto::Entity proto_entity;
  proto_entity.set_name("my entity");
  proto_entity.add_children(child_file_id);
  proto_entity.add_components(transform_file_id);

  proto::TransformComponent proto_transform;
  set_proto_vec(*proto_transform.mutable_translation(), {0, 0, 0});
  set_proto_vec(*proto_transform.mutable_rotation(), {0, 0, 0});
  set_proto_vec(*proto_transform.mutable_scale(), {1, 1, 1});

  proto::Entity proto_child_entity;
  proto_child_entity.set_name("my child entity");
  proto_child_entity.set_parent(entity_file_id);

  proto::ObjectCollection scene;
  auto &objects = *scene.mutable_objects();
  objects[entity_file_id].PackFrom(proto_entity);
  objects[transform_file_id].PackFrom(proto_transform);
  objects[child_file_id].PackFrom(proto_child_entity);


  std::unique_ptr<util::TypeResolver> resolver;
  resolver.reset(util::NewTypeResolverForDescriptorPool(
        kTypeUrlPrefix, DescriptorPool::generated_pool()));

  util::JsonOptions options;
  options.add_whitespace = true;

  std::string result;
  util::BinaryToJsonString(resolver.get(),
                           get_type_url(scene.GetDescriptor()),
                           scene.SerializeAsString(), &result, options);

  DBUG("\n%s", result.c_str());

  // flatbuffers::FlatBufferBuilder fbb;

  // schema::vec3 trans{0, 0, 0}, rot{0, 0, 0}, scale{1.0f, 1.0f, 1.0f};
  // auto transform_location = schema::CreateTransformComponent(fbb, &trans, &rot, &scale);

  // auto entity_file_id = get_file_id();
  // auto transform_file_id = get_file_id();
  // auto transform_object_location = schema::CreateFileObject(fbb, transform_file_id, schema::Object_TransformComponent, transform_location.Union());

  // auto component_ids = fbb.CreateVector<int>({transform_file_id});
  // auto child_ids = fbb.CreateVector<int>({});

  // auto name_location = fbb.CreateString("scene");
  // auto entity_location = schema::CreateEntity(fbb, name_location, component_ids, 0, child_ids);
  // auto entity_object_location = schema::CreateFileObject(fbb, entity_file_id, schema::Object_Entity, entity_location.Union());

  // flatbuffers::Offset<schema::FileObject> file_objects[] = {
  //   entity_object_location,
  //   transform_object_location
  // };

  // auto sorted_file_objects = fbb.CreateVectorOfSortedTables(file_objects, 2);
  // auto object_collection = schema::CreateObjectCollection(fbb, sorted_file_objects);
  // fbb.Finish(object_collection);

  // auto flatbuf = fbb.GetBufferPointer();
  // auto length = fbb.GetSize();
  // scene_entity_buffer = std::vector<uint8_t>{flatbuf, flatbuf + length};


  // flatbuffers::Parser parser;
  // Buffer entity_schema{allocator};
  // bool success;
  // std::tie(entity_schema, success) =
  //   file_util::read_file_to_buffer(allocator,
  //     "../assets/schema/entity.fbs");
  // XASSERT(success, "Could not load schema");

  // czstring<> include_dirs[] = {"../assets/schema/", nullptr};
  // auto parse_result = parser.Parse(c_str(entity_schema), include_dirs);
  // XASSERT(parse_result, "parse error:\n%s", parser.error_.c_str());

  // std::string file_object_json;
  // GenerateText(parser, fbb.GetBufferPointer(), &file_object_json);

  // DBUG("\n%s", file_object_json.c_str());

  // auto &local_field = *fields->LookupByKey("local_position");
  // auto &local_type = *local_field.type();
  // auto index = local_type.index();

  // auto *local_obj = transform_schema.objects()->Get(index);

  // printf("%s\n", local_obj->name()->c_str());

  // auto *local_value = flatbuffers::GetAnyFieldAddressOf<schema::mat4>(transform_table, local_field);

  // printf("%f\n", local_value->col0().y());
  // for (auto &&component_data : *entity_table->components()) {

  // }

  // Buffer entity_schema_buffer{allocator};
  // ReadFile("schema_headers/entity.bfbs", entity_schema_buffer);
  // auto &enity_schema = *reflection::GetSchema(c_str(entity_schema_buffer));

  // auto *root_Table = enity_schema.root_table();
  // auto *fields = root_Table->fields();

  // auto &entity_root = *flatbuffers::GetAnyRoot(fbb.GetBufferPointer());
  // auto &components_field = *fields->LookupByKey("components");

  // auto *component_tables = flatbuffers::GetFieldV<flatbuffers::Table>(entity_root, components_field);

  // auto material = fbb.CreateString("../shaders/blinn_phong.shader");
  // auto mesh_location = schema::CreateMeshComponent(fbb, material);
  // auto mesh_data_location = schema::CreateComponentData(fbb, schema::Component_MeshComponent, mesh_location.Union());

  // flatbuffers::Offset<schema::ComponentData> components[] = { mesh_data_location };
  // auto component_locations = fbb.CreateVector(components, 1);

  // auto entity_location = CreateEntityResource(fbb, component_locations);

  // fbb.Finish(entity_location);

  // {
  //   FileWrite writer("entity_data.bin");
  //   writer.Write(fbb.GetBufferPointer(), fbb.GetSize());
  // }
}

// bool DrawTransform(glm::mat4 &matrix) {
//   bool value_changed = false;

//   glm::vec3 scale;
//   glm::quat rotation;
//   glm::vec3 translation;
//   glm::vec3 skew;
//   glm::vec4 perspective;
//   glm::decompose(matrix, scale, rotation, translation, skew, perspective);

//   value_changed |= DrawVector("position", translation);
//   value_changed |= DrawQuat("rotation", rotation);
//   value_changed |= DrawVector("scale", scale);

//   if (value_changed) {
//     //auto translation_mat = glm::translate(glm::mat4{1.0f}, translation);
//     //auto scale_mat = glm::scale(glm::mat4{1.0f}, scale);
//     //matrix = translation_mat * rotation_mat * scale_mat;
//     //matrix = rotation_mat;
//   }

//   return value_changed;
// }

// void UpdateComponent(const flatbuffers::Table &table) {
//   auto &transform_table = reinterpret_cast<const schema::TransformComponent &>(table);

//   auto &entity_manager = *game_state.injector->get_instance<EntityManager>();
//   auto entity = entity_manager.get(game_state.entity_id);

//   auto &transform_component = *game_state.injector->get_instance<TransformComponent>();
//   auto transform_instance = transform_component.lookup(*entity);

//   auto translation = glm_cast(*transform_table.translation());
//   auto rotation = radians(glm_cast(*transform_table.rotation()));
//   auto scale = glm_cast(*transform_table.scale());

//   auto translation_mat = glm::translate(glm::mat4{1.0f}, translation);
//   auto rotation_mat = glm::eulerAngleYXZ(rotation.y, rotation.x, rotation.z);
//   auto scale_mat = glm::scale(glm::mat4{1.0f}, scale);

//   auto transform = translation_mat * rotation_mat * scale_mat;
//   transform_component.set_local(transform_instance, transform);
// }

// bool DrawVec3(const flatbuffers::Table &table, const reflection::Field &field) {
//   auto &vec3 = *flatbuffers::GetAnyFieldAddressOf<schema::vec3>(table, field);

//   float arr[] = { vec3.x(), vec3.y(), vec3.z() };
//   auto changed = ImGui::DragFloat3(field.name()->c_str(), arr, 0.03f);
//   vec3 = schema::vec3{arr[0], arr[1], arr[2]};

//   return changed;
// }

// bool DrawObj(const reflection::Schema &schema, const flatbuffers::Table &table, const reflection::Field &field) {
//   auto *objects = schema.objects();
//   auto *object = objects->Get(field.type()->index());
//   auto *name = object->name();

//   if (strcmp(name->c_str(), "vec3") == 0) {
//     return DrawVec3(table, field);
//   }

//   return false;
// }

// void DrawComponent(const schema::ComponentData &component_data) {
//   auto &component_table = *reinterpret_cast<const flatbuffers::Table *>(component_data.component());

//   auto component_name = EnumNameComponent(component_data.component_type());
//   ImGui::Text("%s", component_name);

//   auto &schema = *reflection::GetSchema(componet_map[component_name]);
//   auto &fields = *schema.root_table()->fields();

//   bool changed = false;
//   for (auto &&field : fields) {
//     auto *type = field->type();
//     switch(type->base_type()) {
//      case reflection::Obj:
//       changed |= DrawObj(schema, component_table, *field);
//       break;
//     }
//   }

//   if (changed) {
//     UpdateComponent(component_table);
//   }

//   ImGui::Separator();
// }

// void DrawEntity(const schema::Entity &entity_table) {
//   auto *component_data_list = entity_table.components();
//   if (component_data_list != nullptr) {
//     for (auto &&component_data : *component_data_list) {
//       DrawComponent(*component_data);
//     }
//   }
// }

// using EntityPiv =
//   flatbuffers::pointer_inside_vector<schema::Entity, uint8_t>;

// void AddChild(EntityPiv &entity_piv) {
//   auto &schema = *reflection::GetSchema(entity_bfbs);
//   auto root_table = schema.root_table();
//   auto fields = root_table->fields();
//   auto &children_field = *fields->LookupByKey("children");

//   auto entity_root =
//     flatbuffers::piv(
//       reinterpret_cast<flatbuffers::Table *>(*entity_piv), scene_entity_buffer);

//   auto resizing_children =
//     flatbuffers::piv(
//       flatbuffers::GetFieldV<flatbuffers::Offset<schema::Entity>>(**entity_root, children_field),
//       scene_entity_buffer);

//   XASSERT(*resizing_children != nullptr, "children field does not exist");

//   auto new_size = resizing_children->size() + 1;
//   // It's a vector of 2 strings, to which we add one more, initialized to
//   // offset 0.
//   flatbuffers::ResizeVector<flatbuffers::Offset<schema::Entity>>(
//         schema, new_size, 0, *resizing_children, &scene_entity_buffer);
//   // Here we just create a buffer that contans a single string, but this
//   // could also be any complex set of tables and other values.
//   flatbuffers::FlatBufferBuilder entity_fbb;
//   auto entity =
//     schema::CreateEntity(entity_fbb, GetEntityId(),
//       entity_fbb.CreateString("entity"),
//       entity_fbb.CreateVector<flatbuffers::Offset<schema::ComponentData>>(nullptr, 0),
//       entity_fbb.CreateVector<flatbuffers::Offset<schema::Entity>>(nullptr, 0));
//   entity_fbb.Finish(entity);

//   // Add the contents of it to our existing FlatBuffer.
//   // We do this last, so the pointer doesn't get invalidated (since it is
//   // at the end of the buffer):
//   auto entity_ptr = flatbuffers::AddFlatBuffer(scene_entity_buffer,
//                                                entity_fbb.GetBufferPointer(),
//                                                entity_fbb.GetSize());

//   resizing_children->MutateOffset(new_size - 1, entity_ptr);
// }

// void RemoveChild(EntityPiv &parent_piv, EntityPiv &entity_piv) {
//   // auto *children = parent_piv->mutable_children();

//   // auto found = false;
//   // for (auto i = 0; i < children->size() - 1; ++i) {
//   //   auto *child = children->Get(i);
//   //   if (child->id() == entity_piv->id()) {
//   //     found = true;
//   //   }

//   //   if (found) {

//   //   }
//   //   //
//   //   // if (child == nullptr) continue;

//   //   // DBUG("child id %d", child->id());
//   //   // if (child->id() == entity_piv->id()) {
//   //   //   DBUG("Deleting the child with id %d", entity_piv->id());
//   //   //   children->MutateOffset(i, nullptr);
//   //   //   return;
//   //   // }
//   // }
//   // //entity_piv->mutable_children()->MutateOffset(index, nullptr);
// }

// void DrawEntitySelectable(EntityPiv &parent_piv, EntityPiv &entity_piv) {
//   auto name = entity_piv->name();
//   ImGui::Selectable(name->c_str());

//   ImGui::PushID(entity_piv->id());
//   if (ImGui::BeginPopupContextItem("entity context menu"))
//   {
//       if (ImGui::Selectable("Create Child")) {
//         AddChild(entity_piv);
//       }

//       if (*parent_piv != nullptr && ImGui::Selectable("Delete")) {
//         RemoveChild(parent_piv, entity_piv);
//         DBUG("Finished deleting");
//       }

//       ImGui::EndPopup();
//   }
//   ImGui::PopID();
// }

// void DrawHeirarchyEntity(EntityPiv &parent_piv, EntityPiv &entity_piv) {
//   XASSERT(entity_piv->children() != nullptr, "entity must have children vector");

//   if (entity_piv->children()->size() > 0) {
//     auto is_open = ImGui::TreeNode("");
//     ImGui::SameLine();
//     DrawEntitySelectable(parent_piv, entity_piv);

//     if (is_open) {
//       for (auto i = 0; i < entity_piv->mutable_children()->size(); ++i) {
//         auto *child = entity_piv->mutable_children()->Get(i);
//         if (child == nullptr) continue;

//         auto child_piv = flatbuffers::piv(const_cast<schema::Entity *>(child), scene_entity_buffer);
//         DrawHeirarchyEntity(entity_piv, child_piv);
//       }
//       ImGui::TreePop();
//     }
//   } else {
//     ImGui::Indent();
//     DrawEntitySelectable(parent_piv, entity_piv);
//     ImGui::Unindent();
//   }
// }

// void DrawHeirarchy(EntityPiv &entity_piv) {
//   ImGui::Begin("Heirarchy");
//   auto null_parent_piv = EntityPiv{nullptr, scene_entity_buffer};
//   DrawHeirarchyEntity(null_parent_piv, entity_piv);
//   ImGui::End();
// }

extern "C" void UpdateAndRender() {
  static auto prev_time = 0.0;
  auto current_time = glfwGetTime();
  auto delta_time = current_time - prev_time;
  prev_time = current_time;

  glClear(GL_COLOR_BUFFER_BIT);

  ImGuiManager::begin_frame(delta_time);

  ImGuiIO &io = ImGui::GetIO();
  if (io.KeyCtrl && ImGui::IsKeyPressed(GLFW_KEY_Q, false)) {
    glfwSetWindowShouldClose(game_state.window, GL_TRUE);
  }

  project_editor->Draw();

  //auto *scene_entity = schema::GetMutableEntity(scene_entity_buffer.data());
  // auto entity_piv =
  //   flatbuffers::piv(
  //     schema::GetMutableEntity(scene_entity_buffer.data()), scene_entity_buffer);
  // DrawEntity(**entity_piv);
  // DrawHeirarchy(entity_piv);

  static bool show_test_window = true;
  if (show_test_window) {
    ImGui::ShowTestWindow(&show_test_window);
  }

  auto entity_manager = game_state.injector->get_instance<EntityManager>();
  auto entity = entity_manager->get(game_state.entity_id);

  auto transform_component = game_state.injector->get_instance<TransformComponent>();
  auto transform_instance = transform_component->lookup(*entity);

  auto local = transform_component->local(transform_instance);

  auto view_matrix = glm::translate(glm::mat4{1.0f}, glm::vec3{0, -8, -40});
  auto projection_matrix = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.f);

  auto model_view_matrix = view_matrix * local;
  game_state.mv_matrix_uniform->set_value(glm::value_ptr(model_view_matrix));

  auto mvp_matrix = projection_matrix * model_view_matrix;
  game_state.mvp_uniform->set_value(glm::value_ptr(mvp_matrix));

  auto normal_matrix = glm::inverseTranspose(glm::mat3(model_view_matrix));
  game_state.normal_matrix_uniform->set_value(glm::value_ptr(normal_matrix));

  auto material_manager = game_state.injector->get_instance<MaterialManager>();
  material_manager->push_uniforms(*game_state.material);

  auto mesh_component = game_state.injector->get_instance<MeshComponent>();
  mesh_component->render();

  ImGuiManager::end_frame();

  game_state.material->unbind();
}

extern "C" void Shutdown() {
  ImGuiManager::shutdown();
  JobSystem::shutdown();
}
