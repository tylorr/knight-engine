#pragma once

#include "shader_types.h"

struct GLFWwindow;

namespace knight {

namespace ImGuiManager {

void Initialize(GLFWwindow &window, MaterialManager &material_manager);
void Shutdown();

void BeginFrame(double delta_time);
void EndFrame();

void OnMouse(int button, int action);
void OnKey(int key, int action, int mods);
void OnCharacter(unsigned int character);
void OnScroll(double yoffset);

} // namespace ImGuiManager

} // namespace knight
