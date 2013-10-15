#define LOGOG_LEVEL LOGOG_LEVEL_ALL

#include "shader.h"
#include "program.h"
#include "buffer_object.h"
#include "vertex_array.h"
#include "slot_map.h"
#include "work_queue.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <logog.hpp>

using namespace knight;

int CurrentWidth = 800,
    CurrentHeight = 600;

GLFWwindow *window;

bool Initialize();
bool InitWindow();

int main(int argc, char *argv[]) {
  LOGOG_INITIALIZE();
  {
    // 'out' must lose scope before LOGOG_SHUTDOWN()
    logog::Cout out;

    if (Initialize()) {
      Shader vert(ShaderType::VERTEX, "#version 130\nin vec2 position; void main() { gl_Position = vec4(position, 0.0, 1.0); }");
      Shader frag(ShaderType::FRAGMENT, "#version 130\nout vec4 outColor; void main() { outColor = vec4(1.0, 0.0, 0.0, 1.0); }");
      Program program(vert, frag);
      program.Bind();

      float vertices[] = {
        -0.5f,  0.5f,
         0.5f,  0.5f,
         0.5f, -0.5f
      };

      BufferObject vbo(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

      VertexArray vao;
      vao.BindAttribute(vbo, program.GetAttribute("position"), 2, GL_FLOAT,
                        GL_FALSE, 0, (const GLvoid *)0);
      // Main loop
      while (!glfwWindowShouldClose(window)) {

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLES, 0, 3);
        // RenderFunction();

        glfwSwapBuffers(window);
        glfwPollEvents();
      }

      // Close window and terminate GLFW
      glfwTerminate();
    }
  }

  LOGOG_SHUTDOWN();

  // Exit program
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

  INFO("OpenGL Version: %s", glGetString(GL_VERSION));

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);

  return true;
}

bool InitWindow(void) {
  if (!glfwInit()) {
    EMERGENCY("Unable to initialize GLFW library");
    return false;
  }

  window = glfwCreateWindow(CurrentWidth, CurrentHeight, "Hello World", NULL, NULL);

  if (!window) {
    EMERGENCY("Could not create a new rendering window");
    return false;
  }

  glfwMakeContextCurrent(window);

  return true;
}
