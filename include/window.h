#pragma once

#include "common.h"

class Window {
 public:
  static Window *Create(const unsigned int &w, const unsigned int &h);

 private:
  Window(const unsigned int &w, const unsigned int &h);

  GLFWwindow *window_;

  KNIGHT_DISALLOW_COPY_AND_ASSIGN(Window);
};
