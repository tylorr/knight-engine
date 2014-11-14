#define LOGOG_LEVEL LOGOG_LEVEL_ALL

#include "common.h"
#include "shader.h"
#include "shader_program.h"
#include "buffer_object.h"
#include "vertex_array.h"
#include "color_formatter.h"
#include "cout_flush.h"
#include "uniform.h"
#include "uniform_factory.h"
#include "gl_bind.h"
#include "imgui_manager.h"
#include "task_manager.h"
#include "temp_allocator.h"
#include "any_foo.h"
#include "slot_map.h"

#include "monster_generated.h"
#include "event_header_generated.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <logog.hpp>
#include <memory.h>

#include <stb_image.h>
#include <imgui.h>

#include <enet/enet.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h> 

#include <thread>
#include <chrono>
#include <windows.h>
#include <cerrno>
#include <cstring>

using namespace knight;
using namespace knight::events;
using namespace foundation;

int current_width = 1280,
    current_height = 720;

GLFWwindow *window;

UniformFactory uniform_factory;

bool Initialize();
bool InitWindow();

void GlfwKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
void GlfwScrollCallback(GLFWwindow *window, double xoffset, double yoffset);
void GlfwCharCallback(GLFWwindow *window, unsigned int c);

void LoadScript();
void UnloadScript();

typedef void (__cdecl *ScriptInitFunc)(void);
typedef void (__cdecl *ScriptUpdateFunc)(double);
typedef void (__cdecl *ScriptRenderFunc)(void);
typedef void (__cdecl *ScriptShutdownFunc)(void);

std::string script_name = "bin/libscript.dll";

HMODULE script_module;
ScriptInitFunc script_init;
ScriptUpdateFunc script_update;
ScriptRenderFunc script_render;
ScriptShutdownFunc script_shutdown;

void PollNetwork();

ENetAddress address;
ENetHost *server = nullptr;

void StartServer() {
  int result = enet_initialize();
  XASSERT(result == 0, "An error occurred while initializing ENet");

  address.host = ENET_HOST_ANY;
  address.port = 1234;
  server = enet_host_create(&address, 32, 2, 0, 0);

  XASSERT(server != nullptr, "An error occurred while trying to create an ENet server host");
}

void PollNetwork() {
  ENetEvent event;

  while (enet_host_service(server, &event, 0) > 0) {
    switch (event.type) {
      case ENET_EVENT_TYPE_CONNECT:
        INFO("A new client connected from %x:%u.", 
                event.peer->address.host,
                event.peer->address.port);
        /* Store any relevant client information here. */
        event.peer->data = (void *)"Client";
        break;
      case ENET_EVENT_TYPE_RECEIVE:
        {
          auto event_header = GetEventHeader(event.packet->data);
          auto event_type = event_header->event_type();

          if (event_type == Event_Monster) {
            auto monster = reinterpret_cast<const Monster *>(event_header->event());

            INFO("Received monster event mana: %d foo: %d", monster->mana(), monster->foo());
          }

          /* Clean up the packet now that we're done using it. */
          enet_packet_destroy(event.packet);
        }
        break;
      case ENET_EVENT_TYPE_DISCONNECT:
        INFO("%s disconnected.", event.peer->data);
        /* Reset the peer's client information. */
        event.peer->data = NULL;
      case ENET_EVENT_TYPE_NONE:
        break;
    }
  }
}

void ShutdownServer() {
  enet_host_destroy(server);
  enet_deinitialize();
}

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
};

int main(int argc, char *argv[]) {
  
  // line buffering not supported on win32
  setvbuf(stdout, nullptr, _IONBF, BUFSIZ);

  memory_globals::init();

  LOGOG_INITIALIZE();
  {
    logog::CoutFlush out;
    logog::ColorFormatter formatter;
    out.SetFormatter(formatter);

    //Allocator &allocator = memory_globals::default_allocator();

    StartServer();

    if (Initialize()) {
      LoadScript();

      if (script_init != nullptr) {
        script_init();
      }

      UniformFactory uniform_factory;

      Shader vertex_shader;
      vertex_shader.Initialize(ShaderType::VERTEX, GetFileContents("../shaders/simple.vert"));

      Shader fragment_shader;
      fragment_shader.Initialize(ShaderType::FRAGMENT, GetFileContents("../shaders/simple.frag"));

      ShaderProgram shader_program;
      shader_program.Initialize(vertex_shader, fragment_shader, &uniform_factory);

      GlBind<ShaderProgram> shader_program_bind(shader_program);

      auto *mvp_uniform = uniform_factory.Get<float, 4, 4>("MVP");
      auto *mv_matrix_uniform = uniform_factory.Get<float, 4, 4>("ModelView");
      auto *normal_matrix_uniform = uniform_factory.Get<float, 3, 3>("NormalMatrix");

      glm::mat4 model_matrix = glm::mat4(1.0);
      glm::mat4 view_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0, -8, -40));
      glm::mat4 projection_matrix = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.f);

      auto model_view_matrix = view_matrix * model_matrix;
      mv_matrix_uniform->SetValue(glm::value_ptr(model_view_matrix));

      auto mvp_matrix = projection_matrix * model_view_matrix;
      mvp_uniform->SetValue(glm::value_ptr(mvp_matrix));

      auto normal_matrix = glm::inverseTranspose(glm::mat3(model_view_matrix));
      normal_matrix_uniform->SetValue(glm::value_ptr(normal_matrix));

      shader_program.Update();

      Assimp::Importer importer;

      const aiScene* scene = importer.ReadFile("../models/bench.obj", 
              aiProcess_CalcTangentSpace       | 
              aiProcess_Triangulate            |
              aiProcess_JoinIdenticalVertices  |
              aiProcess_SortByPType);

      XASSERT(scene != nullptr, "Could not load bench.obj model");

      std::vector<Vertex> vertices;
      std::vector<unsigned int> indices;
      
      const auto *mesh = scene->mMeshes[0];

      DBUG("Number of vertices %u", mesh->mNumVertices);

      for (unsigned int j = 0; j < mesh->mNumVertices; ++j) {
        const auto &pos = mesh->mVertices[j];
        const auto &normal = mesh->mNormals[j];
        vertices.emplace_back(Vertex{ 
          { pos.x, pos.y, pos.z },
          { normal.x, normal.y, normal.z }
        });
      }

      for (unsigned int j = 0; j < mesh->mNumFaces; ++j) {
        const auto &face = mesh->mFaces[j];
        XASSERT(face.mNumIndices == 3, "Wrong number of indices");

        indices.emplace_back(face.mIndices[0]);
        indices.emplace_back(face.mIndices[1]);
        indices.emplace_back(face.mIndices[2]);
      }

      BufferObject vbo;
      vbo.Initialize(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), &vertices[0], GL_STATIC_DRAW);
      GlBind<BufferObject> vbo_bind(vbo);

      VertexArray vao;
      vao.Initialize();
      GlBind<VertexArray> vao_bind(vao);

      vao.BindAttribute(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), nullptr);
      vao.BindAttribute(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), (void *)sizeof(vertices[0].position));

      BufferObject ibo;
      ibo.Initialize(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
      GlBind<BufferObject> ibo_bind(ibo);

      double current_time;
      double prev_time = 0;
      double delta_time;

      while (!glfwWindowShouldClose(window)) {
        ImGuiManager::BeginFrame();

        current_time = glfwGetTime();
        delta_time = current_time - prev_time;
        prev_time = current_time;

        PollNetwork();

        if (script_update != nullptr) {
          script_update(0);
        }

        glClear(GL_COLOR_BUFFER_BIT);

        if (script_render != nullptr) {
          script_render();
        }

        model_matrix = glm::rotate(model_matrix, (float)delta_time, glm::vec3(0.0f, 1.0f, 0.0f));

        model_view_matrix = view_matrix * model_matrix;
        mv_matrix_uniform->SetValue(glm::value_ptr(model_view_matrix));

        mvp_matrix = projection_matrix * model_view_matrix;
        mvp_uniform->SetValue(glm::value_ptr(mvp_matrix));

        normal_matrix = glm::inverseTranspose(glm::mat3(model_view_matrix));
        normal_matrix_uniform->SetValue(glm::value_ptr(normal_matrix));

        shader_program.Update();

        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);

        ImGuiManager::EndFrame();

        glfwSwapBuffers(window);
        glfwPollEvents();
      }

      if (script_shutdown != nullptr) {
        script_shutdown();
      }

      UnloadScript();

      glfwTerminate();
    }

    ShutdownServer();

  }
  LOGOG_SHUTDOWN();
  memory_globals::shutdown();

  exit(EXIT_SUCCESS);
}

bool Initialize()
{
  GLenum GlewInitResult;

  if (!InitWindow()) {
    return false;
  }

  glewExperimental = GL_TRUE;
  GlewInitResult = glewInit();

  if (GLEW_OK != GlewInitResult) {
    ERR("%s", glewGetErrorString(GlewInitResult));
    return false;
  }

  const GLenum error_value = glGetError();
  if (error_value != GL_NO_ERROR && error_value != GL_INVALID_ENUM) {
    ERR("%s", "Glew init error");
    exit(EXIT_FAILURE);
  }

  INFO("OpenGL Version: %s", glGetString(GL_VERSION));

  ImGuiManager::Initialize(window, &uniform_factory);
  
  glClearColor(0.8f, 0.6f, 0.6f, 1.0f);

  return true;
}

bool InitWindow() {
  if (!glfwInit()) {
    EMERGENCY("Unable to initialize GLFW library");
    return false;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  window = glfwCreateWindow(current_width, current_height, "Hello World", nullptr, nullptr);

  if (!window) {
    EMERGENCY("Could not create a new rendering window");
    return false;
  }

  glfwMakeContextCurrent(window);

  glfwSetKeyCallback(window, GlfwKeyCallback);
  glfwSetScrollCallback(window, GlfwScrollCallback);
  glfwSetCharCallback(window, GlfwCharCallback);

  return true;
}

void LoadScript() {
  script_module = LoadLibraryA(script_name.c_str());
  XASSERT(script_module, "Failed to load script: %s", script_name.c_str());

  script_init = (ScriptInitFunc)GetProcAddress(script_module, "Init");
  script_update = (ScriptUpdateFunc)GetProcAddress(script_module, "Update");
  script_render = (ScriptRenderFunc)GetProcAddress(script_module, "Render");
  script_shutdown = (ScriptShutdownFunc)GetProcAddress(script_module, "Shutdown");
}

void UnloadScript() {
  if (script_module == nullptr) return;

  FreeLibrary(script_module);
  script_module =  nullptr;
  script_init = nullptr;
  script_update = nullptr;
  script_render = nullptr;
  script_shutdown = nullptr;
}

void GlfwKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
  ImGuiManager::OnKey(key, action, mods);
}

void GlfwCharCallback(GLFWwindow *window, unsigned int character) {
  ImGuiManager::OnCharacter(character);
}

void GlfwScrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
  ImGuiManager::OnScroll(yoffset);
}
