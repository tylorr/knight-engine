#ifndef KNIGHT_IMGUI_MANAGER_H_
#define KNIGHT_IMGUI_MANAGER_H_

class GLFWwindow;

namespace knight {

class UniformFactory;

namespace ImGuiManager {

void Initialize(GLFWwindow *window, UniformFactory *uniform_factory);

void BeginFrame();
void EndFrame();

void OnKey(const int &key, const int &action, const int &mods);
void OnCharacter(const unsigned int &character);
void OnScroll(const double &yoffset);

} // namespace ImGuiManager

} // namespace knight

#endif // KNIGHT_IMGUI_MANAGER_H_
