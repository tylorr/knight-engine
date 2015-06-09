#include "game.h"
#include "common.h"
#include "shader_types.h"
#include "uniform.h"
#include "imgui_manager.h"
#include "platform_types.h"
#include "mesh_component.h"
#include "entity_manager.h"
#include "types.h"
#include "std_allocator.h"
#include "material.h"
#include "flatbuffer_allocator.h"

#include "entity_resource_generated.h"
#include "transform_component_generated.h"
#include "mesh_component_generated.h"
#include "types_generated.h"

#include <logog.hpp>
#include <memory.h>

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

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
};

shared_ptr<Material> material;
BufferObject vbo;
BufferObject ibo;
VertexArray vao;

glm::mat4 model_matrix;
Uniform<float, 4, 4> *mvp_uniform;
Uniform<float, 4, 4> *mv_matrix_uniform;
Uniform<float, 3, 3> *normal_matrix_uniform;

shared_ptr<EntityManager> entity_manager;
shared_ptr<MeshComponent> mesh_component;
shared_ptr<MaterialManager> material_manager;

uint32_t index_count;

extern "C" GAME_INIT(Init) {
  auto &alloc = memory_globals::default_allocator();

  material_manager = allocate_shared<MaterialManager>(alloc, alloc);

  ImGuiManager::Initialize(window, *material_manager);

  material = material_manager->CreateMaterial("../shaders/blinn_phong.shader");
  mvp_uniform = material->Get<float, 4, 4>("MVP");
  mv_matrix_uniform = material->Get<float, 4, 4>("ModelView");
  normal_matrix_uniform = material->Get<float, 3, 3>("NormalMatrix");

  model_matrix = glm::mat4{1.0};

  auto importer = Assimp::Importer{};

  const auto scene = importer.ReadFile("../models/bench.obj",
      aiProcess_CalcTangentSpace       | 
      aiProcess_Triangulate            |
      aiProcess_JoinIdenticalVertices  |
      aiProcess_SortByPType);

  XASSERT(scene != nullptr, "Could not load bench.obj model");

  auto mesh = scene->mMeshes[0];

  Array<Vertex> vertices{memory_globals::default_scratch_allocator()};
  for (auto j = 0; j < mesh->mNumVertices; ++j) {
    auto pos = mesh->mVertices[j];
    auto normal = mesh->mNormals[j];
    array::push_back(vertices, 
      Vertex {
        glm::vec3{ pos.x, pos.y, pos.z },
        glm::vec3{ normal.x, normal.y, normal.z }
      });
  }

  Array<unsigned int> indices{memory_globals::default_scratch_allocator()};
  for (auto j = 0; j < mesh->mNumFaces; ++j) {
    auto face = mesh->mFaces[j];
    XASSERT(face.mNumIndices == 3, "Wrong number of indices");

    array::push_back(indices, face.mIndices[0]);
    array::push_back(indices, face.mIndices[1]);
    array::push_back(indices, face.mIndices[2]);
  }

  vao.Initialize();
  vbo.Initialize(GL_ARRAY_BUFFER, array::size(vertices) * sizeof(vertices[0]), &vertices[0], GL_STATIC_DRAW);

  vao.BindAttribute(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), nullptr);
  vao.BindAttribute(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), (void *)sizeof(vertices[0].position));

  ibo.Initialize(GL_ELEMENT_ARRAY_BUFFER, array::size(indices) * sizeof(unsigned int), &(indices)[0], GL_STATIC_DRAW);

  vao.Unbind();
  vbo.Unbind();
  ibo.Unbind();
  material->Unbind();

  entity_manager = allocate_shared<EntityManager>(alloc, alloc);
  auto entity_id = entity_manager->Create();
  auto entity = entity_manager->Get(entity_id);

  mesh_component = allocate_shared<MeshComponent>(alloc, alloc);
  mesh_component->Add(*entity, *material, vao, array::size(indices));

  index_count = array::size(indices);

  mesh_component->GC(*entity_manager);

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

bool show_test_window = true;

char string_buff[256];
char foo_buff[256];

extern "C" GAME_UPDATE_AND_RENDER(UpdateAndRender) {

  static auto prev_time = 0.0;
  auto current_time = glfwGetTime();
  auto delta_time = current_time - prev_time;
  prev_time = current_time;

  glClear(GL_COLOR_BUFFER_BIT);

  ImGuiManager::BeginFrame(delta_time);

  ImGui::Text("Hello, world!");
  ImGui::Text("This one too!");

  ImGui::InputText("string", string_buff, 256);
  ImGui::InputText("foo", foo_buff, 256);

  if (show_test_window) {
    ImGui::ShowTestWindow(&show_test_window);
  }

  model_matrix = glm::rotate(model_matrix, (float)delta_time, glm::vec3(0.0f, 1.0f, 0.0f));

  auto view_matrix = glm::translate(glm::mat4{1.0f}, glm::vec3{0, -8, -40});
  auto projection_matrix = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.f);

  auto model_view_matrix = view_matrix * model_matrix;
  mv_matrix_uniform->SetValue(glm::value_ptr(model_view_matrix));

  auto mvp_matrix = projection_matrix * model_view_matrix;
  mvp_uniform->SetValue(glm::value_ptr(mvp_matrix));

  auto normal_matrix = glm::inverseTranspose(glm::mat3(model_view_matrix));
  normal_matrix_uniform->SetValue(glm::value_ptr(normal_matrix));

  material_manager->PushUniforms(*material);
  
  mesh_component->Render();

  ImGuiManager::EndFrame();

  vao.Unbind();
  material->Unbind();
}

extern "C" GAME_SHUTDOWN(Shutdown) {
  ImGuiManager::Shutdown();
}
