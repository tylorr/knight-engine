#define LOGOG_LEVEL LOGOG_LEVEL_ALL

#include "common.h"
#include "shader_program.h"
#include "buffer_object.h"
#include "vertex_array.h"
#include "color_formatter.h"
#include "cout_flush.h"
#include "uniform.h"
#include "uniform_factory.h"
#include "bind.h"
#include "imgui_manager.h"
#include "udp_listener.h"

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

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h> 

#include <windows.h>

using namespace knight;
using namespace knight::events;
using namespace foundation;

int current_width = 1280,
    current_height = 720;

GLFWwindow *window;

bool Initialize(UniformFactory &);
bool InitWindow();

void GlfwKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
void GlfwScrollCallback(GLFWwindow *window, double xoffset, double yoffset);
void GlfwCharCallback(GLFWwindow *window, unsigned int c);

void LoadScript();
void UnloadScript();

using ScriptInitFunc = void (__cdecl *)(void);
using ScriptUpdateFunc = void (__cdecl *)(double);
using ScriptRenderFunc = void (__cdecl *)(void);
using ScriptShutdownFunc = void (__cdecl *)(void);

std::string script_name = "bin/libscript.dll";

HMODULE script_module;
ScriptInitFunc script_init;
ScriptUpdateFunc script_update;
ScriptRenderFunc script_render;
ScriptShutdownFunc script_shutdown;

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;

  Vertex(const glm::vec3 &position, const glm::vec3 &normal) 
    : position(position), normal(normal) { }
};

int main(int argc, char *argv[]) {
  
  // line buffering not supported on win32
  setvbuf(stdout, nullptr, _IONBF, BUFSIZ);

  memory_globals::init();

  auto logog_init_params = logog::INIT_PARAMS{knight_malloc, knight_free};
  LOGOG_INITIALIZE(&logog_init_params);
  {
    logog::CoutFlush out;
    logog::ColorFormatter formatter;
    out.SetFormatter(formatter);

    Allocator &a = memory_globals::default_allocator();

    auto udp_listener = UdpListener{};
    udp_listener.Start(1234);

    UniformFactory uniform_factory;

    if (Initialize(uniform_factory)) {
      LoadScript();

      if (script_init != nullptr) {
        script_init();
      }

      auto shader_program = ShaderProgram{};
      shader_program.Initialize(uniform_factory, GetFileContents("../shaders/blinn_phong.shader"));

      bind_guard<ShaderProgram> shader_program_bind{shader_program};

      auto mvp_uniform = uniform_factory.Get<float, 4, 4>("MVP");
      auto mv_matrix_uniform = uniform_factory.Get<float, 4, 4>("ModelView");
      auto normal_matrix_uniform = uniform_factory.Get<float, 3, 3>("NormalMatrix");

      auto model_matrix = glm::mat4{1.0};
      auto view_matrix = glm::translate(glm::mat4{1.0f}, glm::vec3{0, -8, -40});
      auto projection_matrix = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.f);

      auto model_view_matrix = view_matrix * model_matrix;
      mv_matrix_uniform->SetValue(glm::value_ptr(model_view_matrix));

      auto mvp_matrix = projection_matrix * model_view_matrix;
      mvp_uniform->SetValue(glm::value_ptr(mvp_matrix));

      auto normal_matrix = glm::inverseTranspose(glm::mat3(model_view_matrix));
      normal_matrix_uniform->SetValue(glm::value_ptr(normal_matrix));

      shader_program.Update();

      auto importer = Assimp::Importer{};

      auto scene = importer.ReadFile("../models/bench.obj", 
              aiProcess_CalcTangentSpace       | 
              aiProcess_Triangulate            |
              aiProcess_JoinIdenticalVertices  |
              aiProcess_SortByPType);

      XASSERT(scene != nullptr, "Could not load bench.obj model");

      std::vector<Vertex> vertices;
      std::vector<unsigned int> indices;
      
      auto mesh = scene->mMeshes[0];

      for (auto j = 0; j < mesh->mNumVertices; ++j) {
        auto pos = mesh->mVertices[j];
        auto normal = mesh->mNormals[j];
        vertices.emplace_back(
          glm::vec3{ pos.x, pos.y, pos.z },
          glm::vec3{ normal.x, normal.y, normal.z }
        );
      }

      for (auto j = 0; j < mesh->mNumFaces; ++j) {
        auto face = mesh->mFaces[j];
        XASSERT(face.mNumIndices == 3, "Wrong number of indices");

        indices.emplace_back(face.mIndices[0]);
        indices.emplace_back(face.mIndices[1]);
        indices.emplace_back(face.mIndices[2]);
      }

      auto vbo = BufferObject{};
      vbo.Initialize(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), &vertices[0], GL_STATIC_DRAW);
      bind_guard<BufferObject> vbo_bind{vbo};

      auto vao = VertexArray{};
      vao.Initialize();
      bind_guard<VertexArray> vao_bind{vao};

      vao.BindAttribute(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), nullptr);
      vao.BindAttribute(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), (void *)sizeof(vertices[0].position));

      auto ibo = BufferObject{};
      ibo.Initialize(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
      bind_guard<BufferObject> ibo_bind{ibo};

      auto current_time = 0.0;
      auto prev_time = 0.0;
      auto delta_time = 0.0;

      while (!glfwWindowShouldClose(window)) {
        ImGuiManager::BeginFrame();

        current_time = glfwGetTime();
        delta_time = current_time - prev_time;
        prev_time = current_time;

        Array<Event> events(a);
        if (udp_listener.Poll(events)) {
          
        }

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

      glfwTerminate();
    }

    udp_listener.Stop();
  }
  LOGOG_SHUTDOWN();

  // Have to let logog shutdown before unloading the script
  UnloadScript();

  memory_globals::shutdown();

  exit(EXIT_SUCCESS);
}

bool Initialize(UniformFactory &uniform_factory)
{
  auto glew_init_result = GLenum{};

  if (!InitWindow()) {
    return false;
  }

  glewExperimental = GL_TRUE;
  glew_init_result = glewInit();

  if (GLEW_OK != glew_init_result) {
    ERR("%s", glewGetErrorString(glew_init_result));
    return false;
  }

  auto error_value = glGetError();
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
