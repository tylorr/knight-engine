#pragma once

#include "shader_types.h"

struct GLFWwindow;

namespace knight {

namespace ImGuiManager {

void Initialize(GLFWwindow *window, UniformManager *uniform_manager);
void Shutdown();

void BeginFrame();
void EndFrame();

void OnMouse(int button, int action);
void OnKey(const int &key, const int &action, const int &mods);
void OnCharacter(const unsigned int &character);
void OnScroll(const double &yoffset);

} // namespace ImGuiManager

} // namespace knight
