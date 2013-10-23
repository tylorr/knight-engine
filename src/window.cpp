#include "window.h"

Window *Window::Create(const unsigned int &w, const unsigned int &h) {
  if (!glfwInit()) {
    EMERGENCY("Unable to initialize GLFW library");
    return false;
  }

  window = glfwCreateWindow(CurrentWidth, CurrentHeight, "Hello World", NULL, NULL);

  if (!window) {
    EMERGENCY("Could not create a new rendering window");
  }

  glfwMakeContextCurrent(window);

  GLenum GlewInitResult;

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
}
