#include "imgui_manager.h"

#include "uniform.h"
#include "material.h"

#include <GL/glew.h>
#include <stb_image.h>
#include <imgui.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <memory.h>

namespace knight {

namespace ImGuiManager {
namespace {

const GLchar *kShaderSource =
  "#if defined(VERTEX)\n"
  "uniform mat4 projection;"
  "layout(location = 0) in vec2 position;"
  "layout(location = 1) in vec2 uv;"
  "layout(location = 2) in vec4 color;"
  "out vec4 frag_color;"
  "out vec2 frag_uv;"
  "void main() {"
  "  frag_uv = uv;"
  "  frag_color = color;"
  "  gl_Position = projection * vec4(position.xy, 0.0f, 1.0f);"
  "}\n"
  "#endif\n"
  "#if defined(FRAGMENT)\n"
  "uniform sampler2D Texture;"
  "in vec2 frag_uv;"
  "in vec4 frag_color;"
  "layout(location = 0) out vec4 out_color;"
  "void main() { "
  "  out_color = texture(Texture, frag_uv) * frag_color;"
  "}\n"
  "#endif\n";

struct ImGuiManagerState {
  MaterialManager *material_manager;
  GLFWwindow *window;

  std::shared_ptr<Material> material;
  VertexArray vao;
  BufferObject vbo;
  size_t buffer_size = 20000;
  GLuint font_texture_handle;
  Uniform<float, 4, 4> *projection_uniform;

  bool mouse_pressed[2] = { false, false };
};

ImGuiManagerState imgui_manager_state;

void RenderDrawLists(ImDrawList **const cmd_lists, int command_lists_count);

const char *GetClipboardString();
void SetClipboardString(const char *text);

void RenderDrawLists(ImDrawList **const command_lists, int command_lists_count) {
  if (command_lists_count == 0) {
    return;
  }

  glEnable(GL_BLEND);
  glBlendEquation(GL_FUNC_ADD);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_SCISSOR_TEST);

  auto material = imgui_manager_state.material;
  material->Bind();

  GL(glActiveTexture(GL_TEXTURE0));
  GL(glBindTexture(GL_TEXTURE_2D, imgui_manager_state.font_texture_handle));
  auto texture_loc = GLint{};
  GL(texture_loc = glGetUniformLocation(material->program_handle(), "Texture"));
  GL(glUniform1i(texture_loc, 0));

  const auto width = ImGui::GetIO().DisplaySize.x;
  const auto height = ImGui::GetIO().DisplaySize.y;
  const auto projection = glm::ortho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
  imgui_manager_state.projection_uniform->SetValue(glm::value_ptr(projection));

  imgui_manager_state.material_manager->PushUniforms(*material);

  auto total_vertex_count = 0_z;
  for (int n = 0; n < command_lists_count; n++) {
    total_vertex_count += command_lists[n]->vtx_buffer.size();
  }

  auto &vbo = imgui_manager_state.vbo;
  vbo.Bind();

  // Grow buffer if too small
  auto needed_buffer_size = total_vertex_count * sizeof(ImDrawVert);
  if (needed_buffer_size > imgui_manager_state.buffer_size) {
      imgui_manager_state.buffer_size = needed_buffer_size + 5000;  
      vbo.Data(imgui_manager_state.buffer_size, nullptr, GL_DYNAMIC_DRAW);
  }

  char *buffer_data;
  GL(buffer_data = static_cast<char *>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY)));
  if (!buffer_data)
    return;

  for (int i = 0; i < command_lists_count; i++) {
    auto *command_list = command_lists[i];
    memcpy(buffer_data, 
           &command_list->vtx_buffer[0], 
           command_list->vtx_buffer.size() * sizeof(ImDrawVert));
    buffer_data += command_list->vtx_buffer.size() * sizeof(ImDrawVert);
  }

  GL(glUnmapBuffer(GL_ARRAY_BUFFER));
  vbo.Unbind();

  auto &vao = imgui_manager_state.vao;
  vao.Bind();

  auto previous_vertex_offset = 0;
  for (auto i = 0; i < command_lists_count; i++) {
    auto  *command_list = command_lists[i];
    auto vertex_offset = previous_vertex_offset;
    const ImDrawCmd* pcmd_end = command_list->commands.end();
    for (const ImDrawCmd* pcmd = command_list->commands.begin(); pcmd != pcmd_end; pcmd++) {
      GL(glScissor(pcmd->clip_rect.x, 
                height - pcmd->clip_rect.w, 
                pcmd->clip_rect.z - pcmd->clip_rect.x, 
                pcmd->clip_rect.w - pcmd->clip_rect.y));
      GL(glDrawArrays(GL_TRIANGLES, vertex_offset, pcmd->vtx_count));
      vertex_offset += pcmd->vtx_count;
    }
    previous_vertex_offset = vertex_offset;
  }

  // Cleanup GL state
  vao.Unbind();
  material->Unbind();
  glDisable(GL_SCISSOR_TEST);
  glBindTexture(GL_TEXTURE_2D, 0);
}

const char *GetClipboardString() {
  return glfwGetClipboardString(imgui_manager_state.window);
}

void SetClipboardString(const char *text) {
  glfwSetClipboardString(imgui_manager_state.window, text);
}

} // namespace

void Initialize(GLFWwindow &window, MaterialManager &material_manager) {
  imgui_manager_state.window = &window;
  imgui_manager_state.material_manager = &material_manager;

  ImGuiIO &io = ImGui::GetIO();

  io.DeltaTime = 1.0f / 60.0f;
  io.PixelCenterOffset = 0.0f;
  io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
  io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
  io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
  io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
  io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
  io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
  io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
  io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
  io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
  io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
  io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
  io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
  io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
  io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
  io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
  io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
  io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

  io.RenderDrawListsFn = RenderDrawLists;
  io.GetClipboardTextFn = GetClipboardString;
  io.SetClipboardTextFn = SetClipboardString;

  GL(glGenTextures(1, &imgui_manager_state.font_texture_handle));
  GL(glBindTexture(GL_TEXTURE_2D, imgui_manager_state.font_texture_handle));
  GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
  GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

  const void *png_data;
  unsigned int png_size;
  ImGui::GetDefaultFontData(nullptr, nullptr, &png_data, &png_size);

  int tex_x, tex_y;
  void *tex_data = stbi_load_from_memory((const unsigned char *)png_data, 
                                         (int)png_size, 
                                         &tex_x, &tex_y, 
                                         nullptr, 0);
  GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_x, tex_y, 
               0, GL_RGBA, GL_UNSIGNED_BYTE, tex_data));
  stbi_image_free(tex_data);

  auto program_handle = imgui_manager_state.material_manager->CreateShaderFromSource("imgui_shader", kShaderSource);
  auto material = imgui_manager_state.material_manager->CreateMaterial(program_handle);
  imgui_manager_state.material = material;

  imgui_manager_state.projection_uniform = material->Get<float, 4, 4>("projection");

  auto &vbo = imgui_manager_state.vbo;
  vbo.Initialize(GL_ARRAY_BUFFER, imgui_manager_state.buffer_size, nullptr, GL_DYNAMIC_DRAW);

  auto &vao = imgui_manager_state.vao;
  vao.Initialize();

  vao.BindAttribute(0, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid *)offsetof(ImDrawVert, pos));
  vao.BindAttribute(1, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid *)offsetof(ImDrawVert, uv));
  vao.BindAttribute(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid *)offsetof(ImDrawVert, col));

  vao.Unbind();
  vbo.Unbind();
  material->Unbind();
}

void BeginFrame(double delta_time) {
  ImGuiIO &io = ImGui::GetIO();

  int display_w, display_h;
  glfwGetFramebufferSize(imgui_manager_state.window, &display_w, &display_h);
  io.DisplaySize = ImVec2((float)display_w, (float)display_h); 

  io.DeltaTime = (float)delta_time;

  int w, h;
  glfwGetWindowSize(imgui_manager_state.window, &w, &h);

  double mouse_x, mouse_y;
  glfwGetCursorPos(imgui_manager_state.window, &mouse_x, &mouse_y);
  mouse_x *= (float)display_w / w;                                                               // Convert mouse coordinates to pixels
  mouse_y *= (float)display_h / h;
  io.MousePos = ImVec2((float)mouse_x, (float)mouse_y);                                          // Mouse position, in pixels (set to -1,-1 if no mouse / on another screen, etc.)
  io.MouseDown[0] = imgui_manager_state.mouse_pressed[0] || glfwGetMouseButton(imgui_manager_state.window, GLFW_MOUSE_BUTTON_LEFT) != 0;  // If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
  io.MouseDown[1] = imgui_manager_state.mouse_pressed[1] || glfwGetMouseButton(imgui_manager_state.window, GLFW_MOUSE_BUTTON_RIGHT) != 0;

  ImGui::NewFrame();
}

void EndFrame() {
  ImGui::Render();

  ImGuiIO &io = ImGui::GetIO();
  io.MouseWheel = 0;

  imgui_manager_state.mouse_pressed[0] = false;
  imgui_manager_state.mouse_pressed[1] = false;
}

void OnMouse(int button, int action) {
  if (action == GLFW_PRESS && button >= 0 && button < 2) {
    imgui_manager_state.mouse_pressed[button] = true;
  }
}

void OnKey(const int &key, const int &action, const int &mods) {
  ImGuiIO &io = ImGui::GetIO();
  if (action == GLFW_PRESS) {
    io.KeysDown[key] = true;
  }
  
  if (action == GLFW_RELEASE) {
    io.KeysDown[key] = false;
  }

  io.KeyCtrl = (mods & GLFW_MOD_CONTROL) != 0;
  io.KeyShift = (mods & GLFW_MOD_SHIFT) != 0;
}

void OnCharacter(const unsigned int &character) {
  if (character > 0 && character <= 255) {
    ImGui::GetIO().AddInputCharacter(character);
  }
}

void OnScroll(const double &yoffset) {
  ImGuiIO &io = ImGui::GetIO();
  io.MouseWheel = (float)yoffset;
}

void Shutdown() {
  //NOTE: TR Some reason if I don't do this I get a segfault from the 
  // destructor of shared_ptr
  imgui_manager_state.material.reset();
}

} // namespace ImGuiManager

} // namespace knight
