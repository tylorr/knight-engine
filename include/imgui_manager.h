#pragma once

#include "shader_types.h"

struct GLFWwindow;

namespace knight {

namespace ImGuiManager {

void initialize(GLFWwindow &window, MaterialManager &material_manager);
void shutdown();

void begin_frame(double delta_time);
void end_frame();

void on_mouse(int button, int action);
void on_key(int key, int action, int mods);
void on_character(unsigned int character);
void on_scroll(double yoffset);

} // namespace ImGuiManager

} // namespace knight
