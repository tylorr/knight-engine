#include "game.h"
#include "game_platform.h"
#include "game_memory.h"
#include "common.h"
#include "shader_types.h"
#include "uniform.h"
#include "imgui_manager.h"
#include "platform_types.h"
#include "mesh_component.h"
#include "entity_manager.h"
#include "types.h"
#include "pointers.h"
#include "material.h"
#include "flatbuffer_allocator.h"
#include "buffer_object.h"

#include "entity_resource_generated.h"
#include "transform_component_generated.h"
#include "mesh_component_generated.h"
#include "types_generated.h"

#include <logog.hpp>
#include <string_stream.h>

#include <imgui.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <cstdio>

using namespace knight;
using namespace foundation;

using std::shared_ptr;
using std::unique_ptr;

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
};

auto EntityManagerFactory() {
  auto &allocator = game_memory::default_allocator();
  return allocate_unique<EntityManager>(allocator, allocator);
}

auto MaterialManagerFactory() {
  auto &allocator = game_memory::default_allocator();
  return allocate_unique<MaterialManager>(allocator, allocator);
}

auto MeshComponentFactory() {
  auto &allocator = game_memory::default_allocator();
  return allocate_unique<MeshComponent>(allocator, allocator);
}

void BuildInjector(GameState &game_state) {
  auto &allocator = game_memory::default_allocator();
  di::InjectorConfig config;

  config.Add(EntityManagerFactory);
  config.Add(MaterialManagerFactory);
  config.Add(MeshComponentFactory);

  game_state.injector = allocate_unique<di::Injector>(allocator, config.BuildInjector(allocator));
}

extern "C" GAME_INIT(Init) {
  GameState *game_state;
  game_memory::Initialize(game_memory, &game_state);

  auto &allocator = game_memory::default_allocator();
  auto &scratch_allocator = game_memory::default_scratch_allocator();

  BuildInjector(*game_state);

  auto material_manager = game_state->injector->get_instance<MaterialManager>();

  ImGuiManager::Initialize(window, *material_manager);

  game_state->material = material_manager->CreateMaterial("../shaders/blinn_phong.shader");

  auto material = game_state->material;

  game_state->mvp_uniform = material->Get<float, 4, 4>("MVP");
  game_state->mv_matrix_uniform = material->Get<float, 4, 4>("ModelView");
  game_state->normal_matrix_uniform = material->Get<float, 3, 3>("NormalMatrix");

  game_state->model_matrix = glm::mat4{1.0};

  auto importer = Assimp::Importer{};

  const auto scene = importer.ReadFile("../models/bench.obj",
      aiProcess_CalcTangentSpace       | 
      aiProcess_Triangulate            |
      aiProcess_JoinIdenticalVertices  |
      aiProcess_SortByPType);

  XASSERT(scene != nullptr, "Could not load bench.obj model");

  auto mesh = scene->mMeshes[0];

  Array<Vertex> vertices{scratch_allocator};
  for (auto j = 0u; j < mesh->mNumVertices; ++j) {
    auto pos = mesh->mVertices[j];
    auto normal = mesh->mNormals[j];
    array::push_back(vertices, 
      Vertex {
        glm::vec3{ pos.x, pos.y, pos.z },
        glm::vec3{ normal.x, normal.y, normal.z }
      });
  }

  Array<unsigned int> indices{scratch_allocator};
  for (auto j = 0u; j < mesh->mNumFaces; ++j) {
    auto face = mesh->mFaces[j];
    XASSERT(face.mNumIndices == 3, "Wrong number of indices");

    array::push_back(indices, face.mIndices[0]);
    array::push_back(indices, face.mIndices[1]);
    array::push_back(indices, face.mIndices[2]);
  }

  game_state->vbo = allocate_unique<BufferObject>(allocator, BufferObject::Target::Array);
  game_state->ibo = allocate_unique<BufferObject>(allocator, BufferObject::Target::ElementArray);

  game_state->vao.Initialize();

  auto &vbo = *game_state->vbo;
  auto &ibo = *game_state->ibo;

  vbo.Initialize(array::size(vertices) * sizeof(vertices[0]), &vertices[0], GL_STATIC_DRAW);

  game_state->vao.BindAttribute(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), nullptr);
  game_state->vao.BindAttribute(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), (void *)sizeof(vertices[0].position));

  ibo.Initialize(
    array::size(indices) * sizeof(unsigned int), 
    array::begin(indices), 
    GL_STATIC_DRAW);

  game_state->vao.Unbind();
  vbo.Unbind();
  ibo.Unbind();
  game_state->material->Unbind();

  auto entity_manager = game_state->injector->get_instance<EntityManager>();
  auto entity_id = entity_manager->Create();
  auto entity = entity_manager->Get(entity_id);

  auto mesh_component = game_state->injector->get_instance<MeshComponent>();
  mesh_component->Add(*entity, *game_state->material, game_state->vao, array::size(indices));
  mesh_component->GC(*entity_manager);

  game_state->index_count = array::size(indices);

  // FlatBufferAllocator fb_alloc(alloc);

  // flatbuffers::FlatBufferBuilder fbb(1024, &fb_alloc);

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

extern "C" GAME_UPDATE_AND_RENDER(UpdateAndRender) {
  GameState *game_state;
  game_memory::Initialize(game_memory, &game_state);

  static auto prev_time = 0.0;
  auto current_time = glfwGetTime();
  auto delta_time = current_time - prev_time;
  prev_time = current_time;

  glClear(GL_COLOR_BUFFER_BIT);

  ImGuiManager::BeginFrame(delta_time);

  ImGui::Text("Hello, world!");
  ImGui::Text("This one too!");

  ImGui::InputText("string", game_state->string_buff, 256);
  ImGui::InputText("foo", game_state->foo_buff, 256);

  static bool show_test_window = true;

  if (show_test_window) {
    ImGui::ShowTestWindow(&show_test_window);
  }

  game_state->model_matrix = glm::rotate(game_state->model_matrix, (float)delta_time, glm::vec3(0.0f, 1.0f, 0.0f));

  auto view_matrix = glm::translate(glm::mat4{1.0f}, glm::vec3{0, -8, -40});
  auto projection_matrix = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.f);

  auto model_view_matrix = view_matrix * game_state->model_matrix;
  game_state->mv_matrix_uniform->SetValue(glm::value_ptr(model_view_matrix));

  auto mvp_matrix = projection_matrix * model_view_matrix;
  game_state->mvp_uniform->SetValue(glm::value_ptr(mvp_matrix));

  auto normal_matrix = glm::inverseTranspose(glm::mat3(model_view_matrix));
  game_state->normal_matrix_uniform->SetValue(glm::value_ptr(normal_matrix));

  auto material_manager = game_state->injector->get_instance<MaterialManager>();
  material_manager->PushUniforms(*game_state->material);

  auto mesh_component = game_state->injector->get_instance<MeshComponent>();
  mesh_component->Render();

  ImGuiManager::EndFrame();

  game_state->vao.Unbind();
  game_state->material->Unbind();
}

extern "C" GAME_SHUTDOWN(Shutdown) {
  ImGuiManager::Shutdown();

  game_memory::Shutdown();
}
