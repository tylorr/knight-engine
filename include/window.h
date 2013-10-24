#ifndef KNIGHT_WINDOW_H_
#define KNIGHT_WINDOW_H_

#include "common.h"

class Window {
 public:
  static Window *Create(const unsigned int &w, const unsigned int &h);

 private:
  Window(const unsigned int &w, const unsigned int &h);

  GLFWwindow *window_;

  KNIGHT_DISALLOW_COPY_AND_ASSIGN(Window);
};

#endif // KNIGHT_WINDOW_H_
