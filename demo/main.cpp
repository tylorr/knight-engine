#define LOGOG_LEVEL LOGOG_LEVEL_ALL

#include "shader.h"
#include "shader_program.h"
#include "buffer_object.h"
#include "vertex_array.h"
#include "color_formatter.h"
#include "cout_flush.h"
#include "uniform.h"
#include "uniform_factory.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <logog.hpp>

#include <exception>

using namespace knight;

int CurrentWidth = 800,
    CurrentHeight = 600;

GLFWwindow *window;

bool Initialize();
bool InitWindow();

int main(int argc, char *argv[]) {
  LOGOG_INITIALIZE();
  {
    logog::CoutFlush out;
    logog::ColorFormatter formatter;
    out.SetFormatter(formatter);

    if (Initialize()) {
      try {
        UniformFactory uniform_factory;

        Shader vert(ShaderType::VERTEX);
        vert.Initialize("#version 330\nin vec2 position; void main() { gl_Position = vec4(position, 0.0, 1.0); }");

        Shader frag(ShaderType::FRAGMENT);
        frag.Initialize("#version 330\nout vec4 outColor; uniform vec4 out_color; void main() { outColor = out_color; }");
        
        ShaderProgram shader_program;
        shader_program.Initialize(vert, frag, &uniform_factory);

        shader_program.Bind();

        float out_color[4] = { 0.0f, 1.0f, 0.0f, 1.0f };

        auto *color_uniform = uniform_factory.Get<float, 4>("out_color");
        
        color_uniform->SetValue(out_color);
        shader_program.Update();
        color_uniform->SetValue(out_color);
        shader_program.Update();

        float vertices[] = {
          -0.5f,  0.5f,
           0.5f,  0.5f,
           0.5f, -0.5f
        };

        BufferObject vbo(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        VertexArray vao;
        vao.BindAttribute(vbo, shader_program.GetAttributeLocation("position"), 2, GL_FLOAT,
                          GL_FALSE, 0, (const GLvoid *)0);
        // Main loop
        while (!glfwWindowShouldClose(window)) {

          glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

          glDrawArrays(GL_TRIANGLES, 0, 3);
          // RenderFunction();

          glfwSwapBuffers(window);
          glfwPollEvents();
        }
      } catch(std::exception &e) {
        std::string err_message = std::string("EXCEPTION: ") + e.what();
        CRITICAL(err_message.c_str());
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
