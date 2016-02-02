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

#include "entity_generated.h"
// #include "entity_resource_generated.h"
#include "transform_component_generated.h"
#include "mesh_component_generated.h"
#include "types_generated.h"
#include "schema_binaries.h"


#include <flatbuffers/idl.h>
#include <flatbuffers/reflection.h>

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

using std::shared_ptr;
using std::unique_ptr;

namespace knight {
namespace detail {

  template<>
  struct attribute_traits<glm::vec3> : float_traits, component_traits<3> {};

  template<>
  struct attribute_traits<glm::vec4> : float_traits, component_traits<4> {};

} // namespace detail

namespace schema {
  auto glm_cast(const vec3 &vec) {
    return glm::vec3{vec.x(), vec.y(), vec.z()};
  }

  auto glm_cast(const vec4 &vec) {
    return glm::vec4{vec.x(), vec.y(), vec.z(), vec.w()};
  }

  auto glm_cast(const mat4 &mat) {
    return glm::mat4{
      glm_cast(mat.col0()),
      glm_cast(mat.col1()),
      glm_cast(mat.col2()),
      glm_cast(mat.col3()),
    };
  }
} // namespace shcema

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

std::unordered_map<czstring<>, const unsigned char *> componet_map {
  { "TransformComponent", transform_component_bfbs }
};

GameState game_state;

pointer<FlatBufferAllocator> fb_alloc;
pointer<flatbuffers::FlatBufferBuilder> fbb_ptr;
const schema::Entity *entity_table;

extern "C" void Init(GLFWwindow &window) {
  JobSystem::initialize();

  auto &allocator = memory_globals::default_allocator();
  auto &scratch_allocator = memory_globals::default_scratch_allocator();

  auto &page_allocator = memory_globals::default_page_allocator();

  auto *block = page_allocator.allocate(128_kib);
  //Ensures(block == nullptr);

  BuddyAllocator buddy{block, 1024};

  auto buddy_block0 = buddy.allocate(128);
  auto buddy_block1 = buddy.allocate(128);
  buddy.deallocate(buddy_block1);
  buddy.deallocate(buddy_block0, 128);

  // auto depth = buddy.get_depth(buddy_block);
  // DBUG("depth: %u", depth);

  page_allocator.deallocate(block);


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
  auto entity_id = entity_manager->Create();
  auto entity = entity_manager->Get(entity_id);

  game_state.entity_id = entity_id;

  auto mesh_component = game_state.injector->get_instance<MeshComponent>();
  mesh_component->Add(*entity, *game_state.material, *game_state.vao);

  auto transform_component = game_state.injector->get_instance<TransformComponent>();
  transform_component->Add(*entity);

  fb_alloc = allocate_unique<FlatBufferAllocator>(allocator, allocator);
  fbb_ptr = allocate_unique<flatbuffers::FlatBufferBuilder>(allocator, 1024, fb_alloc.get());

  auto &fbb = *fbb_ptr;

  schema::vec3 trans{0, 0, 0}, rot{0, 0, 0}, scale{1.0f, 1.0f, 1.0f};
  auto transform_location = schema::CreateTransformComponent(fbb, &trans, &rot, &scale);
  auto transform_data_location = schema::CreateComponentData(fbb, schema::Component_TransformComponent, transform_location.Union());

  flatbuffers::Offset<schema::ComponentData> components[] = { transform_data_location };
  auto component_locations = fbb.CreateVector(components, 1);

  auto entity_location = CreateEntity(fbb, component_locations);

  fbb.ForceDefaults(true);
  fbb.Finish(entity_location);

  // flatbuffers::Parser parser;

  // {
  //   FileRead file{"../assets/schema/types.fbs"};
  //   Buffer buf{allocator};
  //   file.Read(buf);
  //   parser.Parse(c_str(buf));
  // }

  // {
  //   FileRead file{"../assets/schema/transform_component.fbs"};
  //   Buffer buf{allocator};
  //   file.Read(buf);
  //   parser.Parse(c_str(buf));
  // }

  // {
  //   FileRead file{"../assets/schema/entity.fbs"};
  //   Buffer buf{allocator};
  //   file.Read(buf);
  //   parser.Parse(c_str(buf));
  // }

  // std::string json;
  // GenerateText(parser, fbb.GetBufferPointer(),
  //              flatbuffers::GeneratorOptions(), &json);

  // printf("%s\n", json.c_str());

  entity_table = schema::GetEntity(fbb.GetBufferPointer());

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

void UpdateComponent(const flatbuffers::Table &table) {
  auto &transform_table = reinterpret_cast<const schema::TransformComponent &>(table);

  auto &entity_manager = *game_state.injector->get_instance<EntityManager>();
  auto entity = entity_manager.Get(game_state.entity_id);

  auto &transform_component = *game_state.injector->get_instance<TransformComponent>();
  auto transform_instance = transform_component.lookup(*entity);

  auto translation = glm_cast(*transform_table.translation());
  auto rotation = radians(glm_cast(*transform_table.rotation()));
  auto scale = glm_cast(*transform_table.scale());

  auto translation_mat = glm::translate(glm::mat4{1.0f}, translation);
  auto rotation_mat = glm::eulerAngleYXZ(rotation.y, rotation.x, rotation.z);
  auto scale_mat = glm::scale(glm::mat4{1.0f}, scale);

  auto transform = translation_mat * rotation_mat * scale_mat;
  transform_component.set_local(transform_instance, transform);
}

bool DrawVec3(const flatbuffers::Table &table, const reflection::Field &field) {
  auto &vec3 = *flatbuffers::GetAnyFieldAddressOf<schema::vec3>(table, field);

  float arr[] = { vec3.x(), vec3.y(), vec3.z() };
  auto changed = ImGui::DragFloat3(field.name()->c_str(), arr, 0.03f);
  vec3 = schema::vec3{arr[0], arr[1], arr[2]};

  return changed;
}

bool DrawObj(const reflection::Schema &schema, const flatbuffers::Table &table, const reflection::Field &field) {
  auto *type = field.type();
  auto index = type->index();

  auto *objects = schema.objects();
  auto *object = objects->Get(index);
  auto *name = object->name();

  if (strcmp(name->c_str(), "vec3") == 0) {
    return DrawVec3(table, field);
  }

  return false;
}

void DrawComponent(const schema::ComponentData &component_data) {
  auto &component_table = *reinterpret_cast<const flatbuffers::Table *>(component_data.component());

  auto component_name = EnumNameComponent(component_data.component_type());
  ImGui::Text("%s", component_name);

  auto &schema = *reflection::GetSchema(componet_map[component_name]);
  auto &fields = *schema.root_table()->fields();

  bool changed = false;
  for (auto &&field : fields) {
    auto *type = field->type();
    switch(type->base_type()) {
     case reflection::Obj:
      changed |= DrawObj(schema, component_table, *field);
      break;
    }
  }

  if (changed) {
    UpdateComponent(component_table);
  }

  ImGui::Separator();
}

void DrawEntity(const schema::Entity &entity_table) {
  auto &component_data_list = *entity_table.components();
  for (auto &&component_data : component_data_list) {
    DrawComponent(*component_data);
  }
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

  // bool tree_open = ImGui::TreeNode((void *)0, "");
  // ImGui::SameLine();

  // static bool selected = false;
  // ImGui::Selectable("models", &selected);
  // if (tree_open)
  // {
  //   ImGui::TreePop();
  // }

  DrawEntity(*entity_table);

  static bool show_test_window = true;
  if (show_test_window) {
    ImGui::ShowTestWindow(&show_test_window);
  }

  auto entity_manager = game_state.injector->get_instance<EntityManager>();
  auto entity = entity_manager->Get(game_state.entity_id);

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
  mesh_component->Render();

  ImGuiManager::end_frame();

  game_state.material->unbind();
}

extern "C" void Shutdown() {
  ImGuiManager::shutdown();
  JobSystem::shutdown();
}
