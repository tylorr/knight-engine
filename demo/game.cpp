#include "game.h"
#include "game_platform.h"
#include "common.h"
#include "shader_types.h"
#include "uniform.h"
#include "imgui_manager.h"
#include "platform_types.h"
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

#include "entity_generated.h"
// #include "entity_resource_generated.h"
#include "transform_component_generated.h"
#include "mesh_component_generated.h"
#include "types_generated.h"

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
#include <vector>
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

  vec4 convert(const glm::vec4 &glm_vec) {
    return { glm_vec.x, glm_vec.y, glm_vec.z, glm_vec.w };
  }

  mat4 convert(const glm::mat4 &glm_mat) {
    return {
      convert(glm_mat[0]),
      convert(glm_mat[1]),
      convert(glm_mat[2]),
      convert(glm_mat[3])
    };
  }

  glm::vec4 convert(const vec4 &vec) {
    return glm::vec4{vec.x(), vec.y(), vec.z(), vec.w()};
  }

  glm::mat4 convert(const mat4 &mat) {
    return glm::mat4{
      convert(mat.col0()),
      convert(mat.col1()),
      convert(mat.col2()),
      convert(mat.col3()),
    };
  }

} // namespace shcema

} // namespace knight

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
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

  config.Add(EntityManagerFactory);
  config.Add(MaterialManagerFactory);
  config.Add(MeshComponentFactory);
  config.Add(TransformComponentFactory);

  game_state.injector = allocate_unique<di::Injector>(allocator, config.BuildInjector(allocator));
}

void ReadFile(czstring<> path, Buffer &buffer) {
  FileRead file{path};
  file.Read(buffer);
}

GameState game_state;

//const flatbuffers::Vector<flatbuffers::Offset<reflection::Field>> *fields;
const reflection::Field *field;
flatbuffers::Table *transform_root;

pointer<FlatBufferAllocator> fb_alloc;
pointer<flatbuffers::FlatBufferBuilder> fbb_ptr;



extern "C" GAME_INIT(Init) {

  JobSystem::Initialize();

  auto &allocator = memory_globals::default_allocator();
  auto &scratch_allocator = memory_globals::default_scratch_allocator();

  BuildInjector(game_state);

  auto material_manager = game_state.injector->get_instance<MaterialManager>();

  ImGuiManager::Initialize(window, *material_manager);

  game_state.material = material_manager->CreateMaterial("../assets/shaders/blinn_phong.shader");

  auto material = game_state.material;

  game_state.mvp_uniform = material->Get<float, 4, 4>("MVP");
  game_state.mv_matrix_uniform = material->Get<float, 4, 4>("ModelView");
  game_state.normal_matrix_uniform = material->Get<float, 3, 3>("NormalMatrix");

  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string err = tinyobj::LoadObj(shapes, materials, "../assets/models/bench.obj", "../assets/models/");

  XASSERT(err.empty(), "Error loading model: %s", err.c_str());

  auto &obj_mesh = shapes[0].mesh;

  Array<Vertex> vertices{scratch_allocator};
  for (auto i = 0u; i < obj_mesh.positions.size(); i += 3) {
    array::push_back(vertices,
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

  vbo.SetData(vertices, BufferObject::Usage::StaticDraw);
  ibo.SetData(obj_mesh.indices, BufferObject::Usage::StaticDraw);

  vao.SetCount(obj_mesh.indices.size())
     .SetPrimitive(ArrayObject::Primitive::Triangles)
     .SetIndexBuffer(ibo, 0, ArrayObject::IndexType::UnsignedInt)
     .AddVertexBuffer(vbo, 0, Attribute<glm::vec3>{0}, Attribute<glm::vec3>{1});

  auto entity_manager = game_state.injector->get_instance<EntityManager>();
  auto entity_id = entity_manager->Create();
  auto entity = entity_manager->Get(entity_id);

  game_state.entity_id = entity_id;

  auto mesh_component = game_state.injector->get_instance<MeshComponent>();
  mesh_component->Add(*entity, *game_state.material, *game_state.vao);

  auto transform_component = game_state.injector->get_instance<TransformComponent>();
  transform_component->Add(*entity);

  auto transform_instance = transform_component->Lookup(*entity);
  auto local = transform_component->local(transform_instance);

  auto schema_local = schema::convert(local);

  fb_alloc = allocate_unique<FlatBufferAllocator>(allocator, allocator);
  fbb_ptr = allocate_unique<flatbuffers::FlatBufferBuilder>(allocator, 1024, fb_alloc.get());

  auto &fbb = *fbb_ptr;

  auto transform_location = schema::CreateTransformComponent(fbb, &schema_local);
  auto transform_data_location = schema::CreateComponentData(fbb, schema::Component_TransformComponent, transform_location.Union());

  flatbuffers::Offset<schema::ComponentData> components[] = { transform_data_location };
  auto component_locations = fbb.CreateVector(components, 1);

  auto entity_location = CreateEntity(fbb, component_locations);

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

  // auto *entity_table = schema::GetEntity(fbb.GetBufferPointer());
  // auto *schema_component_data = entity_table->components()->Get(0);
  // auto &transform_root = *reinterpret_cast<const flatbuffers::Table *>(schema_component_data->component());

  // auto *entity_table = schema::GetEntity(fbb.GetBufferPointer());
  // auto *schema_component_data = entity_table->components()->Get(0);
  // transform_root = const_cast<flatbuffers::Table *>(reinterpret_cast<const flatbuffers::Table *>(schema_component_data->component()));

  // Buffer buffer{allocator};
  // ReadFile("schema_headers/transform_component.bfbs", buffer);
  // auto &transform_schema = *reflection::GetSchema(c_str(buffer));

  // auto *root_table = transform_schema.root_table();
  // auto *fields = root_table->fields();
  // field = fields->Get(0);

  // auto &local_field = *fields->LookupByKey("local_position");
  // auto &local_type = *local_field.type();
  // auto index = local_type.index();

  // auto *local_obj = transform_schema.objects()->Get(index);

  // printf("%s\n", local_obj->name()->c_str());

  // auto *local_value = flatbuffers::GetAnyFieldAddressOf<schema::mat4>(transform_root, local_field);

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

  // std::string json;
  // GenerateText(parser, fbb.GetBufferPointer(),
  //              flatbuffers::GeneratorOptions(), &json);

  //printf("%s\n", json.c_str());

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

bool DrawVector(czstring<> name, glm::vec3 &vector) {
  return ImGui::DragFloat3(name, &vector[0], 0.5f);
}

bool DrawQuat(czstring<> name, glm::quat &quat, glm::vec3 &euler_offset) {
  auto euler = eulerAngles(quat);
  euler -= euler_offset;

  auto euler_deg = degrees(euler);
  if (DrawVector(name, euler_deg)) {
    euler = radians(euler_deg);

    auto xrot = angleAxis(euler.x, glm::highp_vec3{1.0f, 0.0f, 0.0f});
    auto yrot = angleAxis(euler.y, glm::highp_vec3{0.0f, 1.0f, 0.0f});
    auto zrot = angleAxis(euler.z, glm::highp_vec3{0.0f, 0.0f, 1.0f});
    quat = yrot  * xrot * zrot;

    auto new_euler = eulerAngles(quat);
    euler_offset = new_euler - euler;
    return true;
  }

  return false;
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

// void DrawTransform(flatbuffers::Table *table, const reflection::Field *field) {
//   auto &schema_matrix = *flatbuffers::GetAnyFieldAddressOf<schema::mat4>(*table, *field);
//   auto matrix = convert(schema_matrix);
//   if (DrawTransform(matrix)) {
//     schema_matrix = schema::convert(matrix);

//     auto entity_manager = game_state.injector->get_instance<EntityManager>();
//     auto entity = entity_manager->Get(game_state.entity_id);

//     auto transform_component = game_state.injector->get_instance<TransformComponent>();
//     auto transform_instance = transform_component->Lookup(*entity);
//     transform_component->set_local(transform_instance, matrix);
//   }
// }

glm::quat rotation;
glm::vec3 euler_offset;

extern "C" GAME_UPDATE_AND_RENDER(UpdateAndRender) {
  static auto prev_time = 0.0;
  auto current_time = glfwGetTime();
  auto delta_time = current_time - prev_time;
  prev_time = current_time;

  glClear(GL_COLOR_BUFFER_BIT);

  ImGuiManager::BeginFrame(delta_time);

  // bool tree_open = ImGui::TreeNode((void *)0, "");
  // ImGui::SameLine();

  // static bool selected = false;
  // ImGui::Selectable("models", &selected);
  // if (tree_open)
  // {
  //   ImGui::TreePop();
  // }

  // ImGui::Text("Hello, world!");
  // ImGui::Text("This one too!");

  // ImGui::InputText("string", game_state.string_buff, 256);
  // ImGui::InputText("foo", game_state.foo_buff, 256);

  //for (const auto &field : *fields) {
    //DrawTransform(transform_root, field);

  //}

  //DrawTransform(transform_root, field);

  DrawQuat("rotation", rotation, euler_offset);

  static bool show_test_window = true;

  if (show_test_window) {
    ImGui::ShowTestWindow(&show_test_window);
  }

  auto entity_manager = game_state.injector->get_instance<EntityManager>();
  auto entity = entity_manager->Get(game_state.entity_id);

  auto transform_component = game_state.injector->get_instance<TransformComponent>();
  auto transform_instance = transform_component->Lookup(*entity);

  auto local = transform_component->local(transform_instance);
  local = mat4_cast(rotation);

  // auto rotation_radions = radians(rotation);

  // DrawTransform(local);

  // transform_component->set_local(transform_instance, local);

  //local = glm::eulerAngleYXZ(rotation_radions.y, rotation_radions.x, rotation_radions.z);
  // local = glm::rotate(local, (float)delta_time, glm::vec3(0.0f, 1.0f, 0.0f));
  // transform_component->set_local(transform_instance, local);

  auto view_matrix = glm::translate(glm::mat4{1.0f}, glm::vec3{0, -8, -40});
  auto projection_matrix = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.f);

  auto model_view_matrix = view_matrix * local;
  game_state.mv_matrix_uniform->SetValue(glm::value_ptr(model_view_matrix));

  auto mvp_matrix = projection_matrix * model_view_matrix;
  game_state.mvp_uniform->SetValue(glm::value_ptr(mvp_matrix));

  auto normal_matrix = glm::inverseTranspose(glm::mat3(model_view_matrix));
  game_state.normal_matrix_uniform->SetValue(glm::value_ptr(normal_matrix));

  auto material_manager = game_state.injector->get_instance<MaterialManager>();
  material_manager->PushUniforms(*game_state.material);

  auto mesh_component = game_state.injector->get_instance<MeshComponent>();
  mesh_component->Render();

  ImGuiManager::EndFrame();

  game_state.material->Unbind();
}

extern "C" GAME_SHUTDOWN(Shutdown) {
  ImGuiManager::Shutdown();
  JobSystem::Shutdown();
}
