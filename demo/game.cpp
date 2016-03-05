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
#include "assets/proto/transform_component.pb.h"

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
#include <imgui_internal.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/ext.hpp>

#include <google/protobuf/util/json_util.h>
#include <google/protobuf/util/type_resolver.h>
#include <google/protobuf/util/type_resolver_util.h>
#include <google/protobuf/dynamic_message.h>

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
DynamicMessageFactory message_factory;
std::vector<uint8_t> scene_entity_buffer;

Pointer<editor::ProjectEditor> project_editor;

static const char *kTypeUrlPrefix = "type.googleapis.com";

static std::string get_type_url(const Descriptor* message) {
  return std::string(kTypeUrlPrefix) + "/" + message->full_name();
}

int get_file_id() {
  static int current_id = 0;
  return ++current_id;
}

void set_proto_vec(proto::vec3 &proto_vec, const glm::vec3 &vec) {
  proto_vec.set_x(vec.x);
  proto_vec.set_y(vec.y);
  proto_vec.set_z(vec.z);
}

proto::ObjectCollection scene;

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

  message_factory.SetDelegateToGeneratedFactory(true);

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
  proto_entity.set_id(entity_file_id);
  proto_entity.set_name("my entity");
  proto_entity.add_children(child_file_id);
  proto_entity.add_components(transform_file_id);

  proto::TransformComponent proto_transform;
  set_proto_vec(*proto_transform.mutable_translation(), {0, 0, 0});
  set_proto_vec(*proto_transform.mutable_rotation(), {0, 0, 0});
  set_proto_vec(*proto_transform.mutable_scale(), {1, 1, 1});

  proto::Entity proto_child_entity;
  proto_child_entity.set_id(child_file_id);
  proto_child_entity.set_name("my child entity");
  proto_child_entity.set_parent(entity_file_id);

  auto &objects = *scene.mutable_objects();
  objects[entity_file_id].PackFrom(proto_entity);
  objects[transform_file_id].PackFrom(proto_transform);
  objects[child_file_id].PackFrom(proto_child_entity);
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

proto::Entity create_entity() {
  auto file_id = get_file_id();
  proto::Entity entity;
  entity.set_id(file_id);
  entity.set_name("Entity");
  return entity;
}

void store_entity(proto::ObjectCollection &object_collection, const proto::Entity &entity) {
  (*object_collection.mutable_objects())[entity.id()].PackFrom(entity);
}

void create_empty(proto::ObjectCollection &object_collection) {
  store_entity(object_collection, create_entity());
}

void add_empty_child(proto::ObjectCollection &object_collection, proto::Entity &parent) {
  Expects(parent.id());

  auto child = create_entity();
  child.set_parent(parent.id());
  parent.add_children(child.id());
  store_entity(object_collection, child);
}

void remove_entity(proto::ObjectCollection &object_collection, proto::Entity &entity) {
  Expects(entity.id());
}

enum class EntityChange {
  None = 0,
  AddedChild = 1,
  AddedComponent = 2,
  Removed = 3
};

void add_component(proto::ObjectCollection &object_collection, proto::Entity &entity, const Descriptor &descriptor) {
  auto *factorory_message = message_factory.GetPrototype(&descriptor);

  auto component_file_id = get_file_id();
  entity.add_components(component_file_id);

  std::unique_ptr<Message> component_messsage{factorory_message->New()};
  (*object_collection.mutable_objects())[component_file_id].PackFrom(*component_messsage.get());
}

EntityChange draw_entity_selectable(proto::ObjectCollection &object_collection, proto::Entity &entity) {
  Expects(entity.id());

  ImGui::Selectable(entity.name().c_str());

  auto result = EntityChange::None;
  ImGui::PushID(&entity);
  if (ImGui::BeginPopupContextItem("entity context menu"))
  {
      if (ImGui::Selectable("Create Empty Child")) {
        add_empty_child(object_collection, entity);
        result = EntityChange::AddedChild;
      }

      if (ImGui::Selectable("Delete")) {
        remove_entity(object_collection, entity);
        result = EntityChange::Removed;
      }

      // if (ImGui::BeginMenu("Add Component")) {
      //   for (auto &&item : project_editor->component_descriptors()) {
      //     auto &component_name = item.first;
      //     if (ImGui::Selectable(component_name.c_str())) {
      //       auto *descriptor = item.second;
      //       add_component(object_collection, entity, *descriptor);
      //       result = EntityChange::AddedComponent;
      //     }
      //   }
      //   ImGui::EndMenu();
      // }

      ImGui::EndPopup();
  }
  ImGui::PopID();

  return result;
}

bool draw_heirarchy_entity(proto::ObjectCollection &object_collection, int entity_id) {
  Expects(entity_id);
  auto &objects = *object_collection.mutable_objects();

  proto::Entity entity;
  objects[entity_id].UnpackTo(&entity);
  Expects(entity.id() == entity_id);

  auto entity_change = EntityChange::None;
  auto child_changed = false;
  if (entity.children_size() > 0) {
    auto is_open = ImGui::TreeNode(&entity, "");
    ImGui::SameLine();
    entity_change = draw_entity_selectable(object_collection, entity);

    if (entity_change == EntityChange::Removed) {
      return true;
    }

    if (is_open) {
      for (auto &&child_id : *entity.mutable_children()) {
        child_changed |= draw_heirarchy_entity(object_collection, child_id);
      }
      ImGui::TreePop();
    }
  } else {
    ImGui::Indent();
    entity_change = draw_entity_selectable(object_collection, entity);
    ImGui::Unindent();

    if (entity_change == EntityChange::Removed) {
      return true;
    }
  }

  if (entity_change != EntityChange::None) {
    objects[entity_id].PackFrom(entity);
  }

  return false;
}

bool begin_window_context_item(gsl::czstring<> str_id, int mouse_button = 1) {
  if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(mouse_button)) {
    auto window_rect = ImGui::GetCurrentWindow()->Rect();
    if (ImGui::IsMouseHoveringRect(window_rect.Min, window_rect.Max, false)) {
      ImGui::OpenPopup(str_id);
    }
  }
  return ImGui::BeginPopup(str_id);
}

void draw_heirarchy(proto::ObjectCollection &object_collection) {
  ImGui::Begin("Heirarchy");
  if (begin_window_context_item("heirarchy context menu")) {
      if (ImGui::Selectable("Create Empty")) {
        create_empty(object_collection);
      }
      ImGui::EndPopup();
  }

  auto &objects = *object_collection.mutable_objects();
  for (auto &&item : objects) {
    auto &object = item.second;
    if (object.Is<proto::Entity>()) {
      proto::Entity entity;
      object.UnpackTo(&entity);

      if (!entity.parent()) {
        draw_heirarchy_entity(object_collection, item.first);
      }
    }
  }
  ImGui::End();
}

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

// bool DrawVec3(const flatbuffers::Table &table, const reflection::Field &field) {
//   auto &vec3 = *flatbuffers::GetAnyFieldAddressOf<schema::vec3>(table, field);

//   float arr[] = { vec3.x(), vec3.y(), vec3.z() };
//   auto changed = ImGui::DragFloat3(field.name()->c_str(), arr, 0.03f);
//   vec3 = schema::vec3{arr[0], arr[1], arr[2]};

//   return changed;
// }

bool draw_vec3(const FieldDescriptor &field_descriptor, Message &message) {
  auto &descriptor = *message.GetDescriptor();
  auto *x_desc = descriptor.FindFieldByName("x");
  auto *y_desc = descriptor.FindFieldByName("y");
  auto *z_desc = descriptor.FindFieldByName("z");

  auto &reflection = *message.GetReflection();

  float vec_array[] = {
    reflection.GetFloat(message, x_desc),
    reflection.GetFloat(message, y_desc),
    reflection.GetFloat(message, z_desc)
  };
  auto changed = ImGui::DragFloat3(field_descriptor.name().c_str(), vec_array, 0.03f);

  if (changed) {
    reflection.SetFloat(&message, x_desc, vec_array[0]);
    reflection.SetFloat(&message, y_desc, vec_array[1]);
    reflection.SetFloat(&message, z_desc, vec_array[2]);
  }

  return changed;
}

bool draw_message_field(const FieldDescriptor &field_descriptor, Message &message) {
  auto &descriptor = *message.GetDescriptor();

  if (descriptor.full_name() == "knight.proto.vec3") {
    return draw_vec3(field_descriptor, message);
  }

  return false;
}

bool draw_component(Message &component_messsage) {
  auto &descriptor = *component_messsage.GetDescriptor();
  auto &reflection = *component_messsage.GetReflection();
  ImGui::Text("%s", descriptor.name().c_str());

  auto changed = false;
  for (auto i = 0; i < descriptor.field_count(); ++i) {
    auto &field_descriptor = *descriptor.field(i);

    switch (field_descriptor.type()) {
      case FieldDescriptor::TYPE_MESSAGE:
        auto *field_message = reflection.MutableMessage(&component_messsage, &field_descriptor);
        changed |= draw_message_field(field_descriptor, *field_message);
    }
  }

  return changed;
}

void draw_inspector(editor::ProjectEditor &project_editor) {
  ImGui::Begin("Inspector");
  auto *selected_entry = project_editor.selected_entry();
  if (selected_entry != nullptr && selected_entry->type == editor::EntryType::ComponentSchema) {

    auto &importer = project_editor.importer();
    auto *pool = importer.pool();
    for (auto &&item : *selected_entry->resource_handle.mutable_defaults()) {
      auto &default_any = item.second;

      std::string full_name;
      internal::ParseAnyTypeUrl(default_any.type_url(), &full_name);
      auto *descriptor = pool->FindMessageTypeByName(full_name);

      if (descriptor == nullptr) continue;

      auto *factory_component = message_factory.GetPrototype(descriptor);
      std::unique_ptr<Message> component_messsage{factory_component->New()};

      // Message component_messsage;
      default_any.UnpackTo(component_messsage.get());
      if (draw_component(*component_messsage)) {
        default_any.PackFrom(*component_messsage);
      }
    }
  }
  ImGui::End();
}

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

  if (ImGui::Selectable("Print scene")) {
    std::unique_ptr<util::TypeResolver> resolver{
      util::NewTypeResolverForDescriptorPool(
        kTypeUrlPrefix, DescriptorPool::generated_pool())};

    util::JsonOptions options;
    options.add_whitespace = true;

    std::string json;
    util::BinaryToJsonString(resolver.get(),
                             get_type_url(scene.GetDescriptor()),
                             scene.SerializeAsString(), &json, options);

    DBUG("\n%s", json.c_str());
  }

  project_editor->draw();

  draw_heirarchy(scene);

  draw_inspector(*project_editor.get());

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
