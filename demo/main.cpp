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

#include <thread>
#include <chrono>

using namespace knight;
using namespace foundation;
//using namespace knight::events;

int current_width = 1280,
    current_height = 720;

GLFWwindow *window;

UniformFactory uniform_factory;

bool Initialize();
bool InitWindow();

void GlfwKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
void GlfwScrollCallback(GLFWwindow *window, double xoffset, double yoffset);
void GlfwCharCallback(GLFWwindow *window, unsigned int c);

int main(int argc, char *argv[]) {
  memory_globals::init();

  LOGOG_INITIALIZE();
  {
    logog::CoutFlush out;
    logog::ColorFormatter formatter;
    out.SetFormatter(formatter);

    if (Initialize()) {
      // flatbuffers::FlatBufferBuilder fbb;

      // auto monster_location = CreateMonster(fbb, 100, 20);

      // auto event_header_location = CreateEventHeader(fbb, Event_Monster, monster_location.Union());

      // fbb.Finish(event_header_location);

      // auto event_header = GetEventHeader(fbb.GetBufferPointer());

      // auto event_type = event_header->event_type();

      // if (event_type == Event_Monster) {
      //   auto *monster = GetMonster(event_header->event());
      //   DBUG("Monster mana: %d", monster->mana());
      //   DBUG("Monster foo: %d", monster->foo());
      // }            

      // Main loop
      while (!glfwWindowShouldClose(window)) {
        ImGuiManager::BeginFrame();

        static bool show_test_window = true;
        static bool show_another_window = false;
        static float f;
        ImGui::Text("Hello, world!");
        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
        show_test_window ^= ImGui::Button("Test Window");
        show_another_window ^= ImGui::Button("Another Window");

        static float ms_per_frame[120] = { 0 };
        static int ms_per_frame_idx = 0;
        static float ms_per_frame_accum = 0.0f;
        ms_per_frame_accum -= ms_per_frame[ms_per_frame_idx];
        ms_per_frame[ms_per_frame_idx] = ImGui::GetIO().DeltaTime * 1000.0f;
        ms_per_frame_accum += ms_per_frame[ms_per_frame_idx];
        ms_per_frame_idx = (ms_per_frame_idx + 1) % 120;
        const float ms_per_frame_avg = ms_per_frame_accum / 120;
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", ms_per_frame_avg, 1000.0f / ms_per_frame_avg);

        if (show_test_window) {
          ImGui::ShowTestWindow(&show_test_window);
        }

        // Show another simple window
        if (show_another_window) {
          ImGui::Begin("Another Window", &show_another_window, ImVec2(200, 100));
          ImGui::Text("Hello");
          ImGui::End();
        }

        glClear(GL_COLOR_BUFFER_BIT);

        ImGuiManager::EndFrame();

        glfwSwapBuffers(window);
        glfwPollEvents();
      }

      glfwTerminate();
    }
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

  if (error_value != GL_NO_ERROR && error_value != GL_INVALID_ENUM)
  {
    ERR("%s", "Glew init error");
    exit(EXIT_FAILURE);
  }

  ImGuiManager::Initialize(window, &uniform_factory);

  INFO("OpenGL Version: %s", glGetString(GL_VERSION));

  glClearColor(0.8f, 0.6f, 0.6f, 1.0f);

  return true;
}

bool InitWindow() {
  if (!glfwInit()) {
    EMERGENCY("Unable to initialize GLFW library");
    return false;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
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

void GlfwKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
  ImGuiManager::OnKey(key, action, mods);
}

void GlfwCharCallback(GLFWwindow *window, unsigned int character) {
  ImGuiManager::OnCharacter(character);
}

void GlfwScrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
  ImGuiManager::OnScroll(yoffset);
}
