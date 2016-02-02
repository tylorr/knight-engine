#define LOGOG_LEVEL LOGOG_LEVEL_ALL

#include "common.h"
#include "logog_util.h"
#include "imgui_manager.h"
#include "udp_listener.h"
#include "game_code.h"
#include "game_platform.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory.h>
#include <string_stream.h>

#include <logog.hpp>

using namespace knight;
using namespace foundation;
using namespace string_stream;

int current_width = 1280,
    current_height = 720;

GLFWwindow *window;

void Initialize();
void InitWindow();

void GlfwErrorCallback(int error, const char *description);
void GlfwMouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
void GlfwKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
void GlfwScrollCallback(GLFWwindow *window, double xoffset, double yoffset);
void GlfwCharCallback(GLFWwindow *window, unsigned int c);

int main(int argc, char *argv[]) {

  // line buffering not supported on win32
  setvbuf(stdout, nullptr, _IONBF, BUFSIZ);
  std::set_terminate(trace_abort);

  memory_globals::init();

  auto game = GameCode{memory_globals::default_allocator()};

  auto logog_init_params = logog::INIT_PARAMS{knight_malloc, knight_free};
  LOGOG_INITIALIZE(&logog_init_params);
  {
    logog::CoutFlush out;
    logog::ColorFormatter formatter;
    out.SetFormatter(formatter);

    auto udp_listener = UdpListener{};
    udp_listener.Start(1234);

    Initialize();

    auto &page_allocator = memory_globals::default_page_allocator();

    // auto game_memory = GameMemory{};

    // game_memory.temporary_memory_size = 512_mib;
    // game_memory.memory_size = 256_mib + game_memory.temporary_memory_size;
    // game_memory.memory = page_allocator.allocate(game_memory.memory_size);

    // XASSERT(game_memory.memory != nullptr, "Page allocation failed");

    // TODO: TR Get these from cmake
    auto source_dll_name = "bin/libgame.dll";
    auto temp_dll_name = "bin/temp_libgame.dll";
    game_code::Load(game, source_dll_name, temp_dll_name);

    if (game.Init != nullptr) {
      game.Init(*window);
    }

    while (!glfwWindowShouldClose(window)) {
      if(game_code::IsDirty(game)) {
        game_code::Reload(game);
      }

      glfwGetFramebufferSize(window, &current_width, &current_height);
      glViewport(0, 0, current_width, current_height);

      if (game.UpdateAndRender != nullptr) {
        game.UpdateAndRender();
      }

      glfwSwapBuffers(window);
      glfwPollEvents();
    }

    if (game.Shutdown != nullptr) {
      game.Shutdown();
    }

    udp_listener.Stop();

    glfwTerminate();
  }
  LOGOG_SHUTDOWN();
  game_code::Unload(game);
  memory_globals::shutdown();

  return EXIT_SUCCESS;
}

void Initialize() {
  InitWindow();

  glewExperimental = GL_TRUE;
  auto glew_init_result = glewInit();

  XASSERT(glew_init_result == GLEW_OK, "Glew init error:\n%s", glewGetErrorString(glew_init_result));

  // Swallow GL_INVALID_ENUM from glewInit()
  auto error_value = glGetError();
  if (error_value != GL_NO_ERROR && error_value != GL_INVALID_ENUM) {
    XASSERT(error_value != GL_NO_ERROR, "glewInit error: %s", glErrorString(error_value));
  }

  INFO("OpenGL %s", glGetString(GL_VERSION));

  glClearColor(0.8f, 0.6f, 0.6f, 1.0f);
}

OpenglVersion supported_versions[] = {
  { 4, 5 },
  { 4, 4 },
  { 4, 3 },
  { 4, 2 },
  { 4, 1 },
  { 4, 0 },
  { 3, 3 },
};

void InitWindow() {
  glfwSetErrorCallback(GlfwErrorCallback);

  auto init_result = glfwInit();
  XASSERT(init_result, "Unable to initialize GLFW library");

  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  for (auto &&version : supported_versions) {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, version.major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, version.minor);
    window = glfwCreateWindow(current_width, current_height, "Hello World", nullptr, nullptr);
    if (window != nullptr)
      break;
  }

  XASSERT(window != nullptr, "Could not create a new rendering window");

  glfwMakeContextCurrent(window);

  glfwSetMouseButtonCallback(window, GlfwMouseButtonCallback);
  glfwSetKeyCallback(window, GlfwKeyCallback);
  glfwSetScrollCallback(window, GlfwScrollCallback);
  glfwSetCharCallback(window, GlfwCharCallback);

  INFO("GLFW %s", glfwGetVersionString());
}

void GlfwErrorCallback(int error, const char *description) {
  printf("GLFW error:\n%s\n\n", description);
}

void GlfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
  ImGuiManager::OnMouse(button, action);
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
