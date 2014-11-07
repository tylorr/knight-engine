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
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <logog.hpp>
#include <memory.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <imgui.h>

#include <enet/enet.h>

#include <thread>
#include <chrono>
#include <windows.h>
#include <cerrno>
#include <cstring>
#include <atomic>
#include <thread>

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

std::string script_name = "lib/libscript.dll";

HMODULE script_module;
ScriptInitFunc script_init;
ScriptUpdateFunc script_update;
ScriptRenderFunc script_render;
ScriptShutdownFunc script_shutdown;

void PollNetwork();

ENetAddress address;
ENetHost *server = nullptr;
std::thread *server_thread_;
std::atomic_bool stop_server_;

void StartServer() {
  int result = enet_initialize();
  XASSERT(result == 0, "An error occurred while initializing ENet");

  address.host = ENET_HOST_ANY;
  address.port = 1234;
  server = enet_host_create(&address, 32, 2, 0, 0);

  XASSERT(server != nullptr, "An error occurred while trying to create an ENet server host");

  stop_server_ = false;

  Allocator &allocator = memory_globals::default_allocator();
  server_thread_ = allocator.make_new<std::thread>(PollNetwork);
}

void PollNetwork() {
  ENetEvent event;

  while (!stop_server_) {
    /* Wait up to 1000 milliseconds for an event. */
    while (enet_host_service(server, &event, 1000) > 0) {
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

  DBUG("Server stopping...");
}

void ShutdownServer() {
  stop_server_ = true;
  server_thread_->join();

  DBUG("Server stopped");

  Allocator &allocator = memory_globals::default_allocator();
  allocator.make_delete(server_thread_);
  server_thread_ = nullptr;

  enet_host_destroy(server);
  enet_deinitialize();
}

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

      while (!glfwWindowShouldClose(window)) {
        ImGuiManager::BeginFrame();

        if (script_module != nullptr) {
          if (ImGui::Button("Unload Script")) {
            UnloadScript();
          }
        } else {
          if (ImGui::Button("Load Script")) {
            LoadScript();
          }
        }

        if (script_update != nullptr) {
          script_update(0);
        }

        glClear(GL_COLOR_BUFFER_BIT);

        if (script_render != nullptr) {
          script_render();
        }

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
