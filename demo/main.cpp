#define LOGOG_LEVEL LOGOG_LEVEL_ALL

#include "common.h"
#include "color_formatter.h"
#include "cout_flush.h"
#include "shader_types.h"
#include "imgui_manager.h"
#include "udp_listener.h"
#include "task_manager.h"
#include "game.h"

#include "event_header_generated.h"
#include "unload_script_generated.h"

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

#include <temp_allocator.h>

#include <windows.h>

using namespace knight;
using namespace foundation;

struct GameCode {
  HMODULE module;
  FILETIME dll_last_write_time;

  game_init *Init;
  game_update_and_render *UpdateAndRender;
  game_shutdown *Shutdown;
};

int current_width = 1280,
    current_height = 720;

GLFWwindow *window;

bool Initialize(UniformFactory &);
bool InitWindow();

void GlfwKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
void GlfwScrollCallback(GLFWwindow *window, double xoffset, double yoffset);
void GlfwCharCallback(GLFWwindow *window, unsigned int c);

GameCode LoadGameCode(const char *source_dll_name, const char *temp_dll_name);
void UnloadGameCode(GameCode *game_code);

inline FILETIME GetLastWriteTime(const char *filename) {
    FILETIME last_write_time = {};

    WIN32_FILE_ATTRIBUTE_DATA data;
    if(GetFileAttributesEx(filename, GetFileExInfoStandard, &data))
    {
        last_write_time = data.ftLastWriteTime;
    }

    return last_write_time;
}

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

    UniformFactory uniform_factory(a);
    if (Initialize(uniform_factory)) {
      
      std::string source_dll_name = "bin/libgame.dll";
      std::string temp_dll_name = "bin/temp_libgame.dll";
      GameCode game = LoadGameCode(source_dll_name.c_str(), temp_dll_name.c_str());
      
      if (game.Init) {
        game.Init(uniform_factory);
      }

      while (!glfwWindowShouldClose(window)) {
        FILETIME new_dll_write_time = GetLastWriteTime(source_dll_name.c_str());
        if(CompareFileTime(&new_dll_write_time, &game.dll_last_write_time) != 0) {
          UnloadGameCode(&game);
          game = LoadGameCode(source_dll_name.c_str(), temp_dll_name.c_str());
        }

        ImGuiManager::BeginFrame();

        // Array<Event> events(a);
        // if (udp_listener.Poll(events)) {
        //   auto event_header = events[0].header;
        //   auto event_type = event_header->event_type();

        //   if (event_type == events::EventType_UnloadScript) {
        //     //auto monster = reinterpret_cast<const events::UnloadScript *>(event_header->event());
        //     //INFO("Received monster event mana: %d foo: %d", monster->mana(), monster->foo());
        //   }
        // }

        if (game.UpdateAndRender) {
          game.UpdateAndRender();
        }

        ImGuiManager::EndFrame();

        glfwSwapBuffers(window);
        glfwPollEvents();
      }

      if (game.Shutdown) {
        game.Shutdown();
      }
      
      ImGuiManager::Shutdown();

      glfwTerminate();
    }

    udp_listener.Stop();
  }
  LOGOG_SHUTDOWN();

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

GameCode LoadGameCode(const char *source_dll_name, const char *temp_dll_name) {
  GameCode result;

  CopyFile(source_dll_name, temp_dll_name, false);

  result.module = LoadLibraryA(temp_dll_name);

  result.dll_last_write_time = GetLastWriteTime(source_dll_name);

  bool is_valid = false;
  if (result.module) {
    result.Init = (game_init *)GetProcAddress(result.module, "Init");
    result.UpdateAndRender = (game_update_and_render *)GetProcAddress(result.module, "UpdateAndRender");
    result.Shutdown = (game_shutdown *)GetProcAddress(result.module, "Shutdown");
    is_valid = (result.Init && result.UpdateAndRender && result.Shutdown);
  }

  if (!is_valid) {
    result.Init = nullptr;
    result.UpdateAndRender = nullptr;
    result.Shutdown = nullptr;
  }

  return result;
}

void UnloadGameCode(GameCode *game_code) {
  if (game_code->module) {
    FreeLibrary(game_code->module);
    game_code->module = nullptr;
  }

  game_code->Init = nullptr;
  game_code->UpdateAndRender = nullptr;
  game_code->Shutdown = nullptr;
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
