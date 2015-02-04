#include "game.h"
#include "common.h"
#include "shader_types.h"
#include "buffer_object.h"
#include "vertex_array.h"
#include "uniform.h"
#include "uniform_manager.h"
#include "imgui_manager.h"
#include "platform_types.h"

#include <logog.hpp>
#include <memory.h>
#include <string_stream.h>
#include <temp_allocator.h>

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
using namespace foundation::string_stream;

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
};

Allocator *allocator;
UniformManager *uniform_manager_;
ShaderProgram program;
BufferObject vbo;
BufferObject ibo;
VertexArray vao;

glm::mat4 model_matrix;
Uniform<float, 4, 4> *mvp_uniform;
Uniform<float, 4, 4> *mv_matrix_uniform;
Uniform<float, 3, 3> *normal_matrix_uniform;

std::vector<Vertex> vertices;
std::vector<unsigned int> indices;

extern "C" GAME_INIT(Init) {

  ImGuiManager::Initialize(&window, &uniform_manager);

  uniform_manager_ = &uniform_manager;
  allocator = &memory_globals::default_allocator();

  {
    TempAllocator4096 temp_allocator;
    Buffer phong_shader_buffer{temp_allocator};

    File phong_shader{"../shaders/blinn_phong.shader"};
    phong_shader.Read(phong_shader_buffer);
    program.Initialize(uniform_manager, c_str(phong_shader_buffer));
  }

  mvp_uniform = uniform_manager.Get<float, 4, 4>(program, "MVP");
  mv_matrix_uniform = uniform_manager.Get<float, 4, 4>(program, "ModelView");
  normal_matrix_uniform = uniform_manager.Get<float, 3, 3>(program, "NormalMatrix");

  model_matrix = glm::mat4{1.0};

  uniform_manager.PushUniforms(program);

  auto importer = Assimp::Importer{};

  const auto scene = importer.ReadFile("../models/bench.obj", 
      aiProcess_CalcTangentSpace       | 
      aiProcess_Triangulate            |
      aiProcess_JoinIdenticalVertices  |
      aiProcess_SortByPType);

  XASSERT(scene != nullptr, "Could not load bench.obj model");

  auto mesh = scene->mMeshes[0];

  for (auto j = 0; j < mesh->mNumVertices; ++j) {
    auto pos = mesh->mVertices[j];
    auto normal = mesh->mNormals[j];
    vertices.emplace_back(Vertex {
      glm::vec3{ pos.x, pos.y, pos.z },
      glm::vec3{ normal.x, normal.y, normal.z }
    });
  }

  for (auto j = 0; j < mesh->mNumFaces; ++j) {
    auto face = mesh->mFaces[j];
    XASSERT(face.mNumIndices == 3, "Wrong number of indices");

    indices.emplace_back(face.mIndices[0]);
    indices.emplace_back(face.mIndices[1]);
    indices.emplace_back(face.mIndices[2]);
  }

  vao.Initialize();
  vbo.Initialize(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), &vertices[0], GL_STATIC_DRAW);

  vao.BindAttribute(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), nullptr);
  vao.BindAttribute(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), (void *)sizeof(vertices[0].position));

  ibo.Initialize(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

  vao.Unbind();
  vbo.Unbind();
  ibo.Unbind();
  program.Unbind();
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

  program.Bind();
  uniform_manager_->PushUniforms(program);
  
  vao.Bind();

  glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);

  ImGuiManager::EndFrame();

  program.Unbind();
  vao.Unbind();
}

extern "C" GAME_SHUTDOWN(Shutdown) {
  // allocator->make_delete(program);
}
