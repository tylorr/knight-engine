#include "imgui_manager.h"

#include "uniform.h"
#include "material.h"
#include "buffer_object.h"
#include "pointers.h"
#include "array_object.h"
#include "attribute.h"

#include <GL/glew.h>
#include <imgui.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <memory.h>

namespace knight {

namespace detail {

  template<>
  struct attribute_traits<ImVec2> : float_traits, component_traits<2> {};

  template<>
  struct attribute_traits<ImVec4> : float_traits, component_traits<4> {};

}

namespace ImGuiManager {
namespace {

const GLchar *kShaderSource =
  "#if defined(VERTEX)\n"
  "uniform mat4 ProjMtx;\n"
  "layout(location = 0) in vec2 Position;\n"
  "layout(location = 1) in vec2 UV;\n"
  "layout(location = 2) in vec4 Color;\n"
  "out vec2 Frag_UV;\n"
  "out vec4 Frag_Color;\n"
  "void main() {\n"
  "  Frag_UV = UV;\n"
  "  Frag_Color = Color;\n"
  "  gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
  "}\n"
  "#endif\n"
  "#if defined(FRAGMENT)\n"
  "uniform sampler2D Texture;\n"
  "in vec2 Frag_UV;\n"
  "in vec4 Frag_Color;\n"
  "out vec4 Out_Color;\n"
  "void main() {\n"
  "  Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
  "}\n"
  "#endif\n";

struct ImGuiManagerState {
  MaterialManager *material_manager;
  GLFWwindow *window;

  std::shared_ptr<Material> material;
  pointer<ArrayObject> vao;
  pointer<BufferObject> vbo;
  pointer<BufferObject> ibo;
  GLuint font_texture_handle;
  GLint texture_location;
  Uniform<float, 4, 4> *projection_uniform;

  bool mouse_pressed[3] = { false, false, false };
  float mouse_wheel = 0.0f;
};

ImGuiManagerState imgui_manager_state;

const char *GetClipboardString();
void SetClipboardString(const char *text);

void RenderDrawLists(ImDrawData* draw_data) {

  // Backup GL state
  GLint last_program, last_texture, last_array_buffer, last_element_array_buffer, last_vertex_array;
  glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
  glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
  glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &last_element_array_buffer);
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);

  glEnable(GL_BLEND);
  glBlendEquation(GL_FUNC_ADD);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_SCISSOR_TEST);
  glActiveTexture(GL_TEXTURE0);

  // Handle cases of screen coordinates != from framebuffer coordinates (e.g. retina displays)
  ImGuiIO& io = ImGui::GetIO();
  float fb_height = io.DisplaySize.y * io.DisplayFramebufferScale.y;
  draw_data->ScaleClipRects(io.DisplayFramebufferScale);

  const auto projection = glm::ortho(0.0f, io.DisplaySize.x, io.DisplaySize.y, 0.0f, -1.0f, 1.0f);
  imgui_manager_state.projection_uniform->SetValue(glm::value_ptr(projection));

  auto material = imgui_manager_state.material;
  imgui_manager_state.material_manager->PushUniforms(*material);
  GL(glUniform1i(imgui_manager_state.texture_location, 0));

  auto &vao = *imgui_manager_state.vao;
  vao.Bind();

  auto &vbo = *imgui_manager_state.vbo;
  auto &ibo = *imgui_manager_state.ibo;

  for (int n = 0; n < draw_data->CmdListsCount; n++) {
    const ImDrawList* cmd_list = draw_data->CmdLists[n];
    const ImDrawIdx* idx_buffer_offset = 0;

    vbo.SetData({cmd_list->VtxBuffer.begin(), cmd_list->VtxBuffer.size() * sizeof(ImDrawVert)}, BufferObject::Usage::StreamDraw);
    ibo.SetData({cmd_list->IdxBuffer.begin(), cmd_list->IdxBuffer.size() * sizeof(ImDrawIdx)}, BufferObject::Usage::StreamDraw);

    for (const ImDrawCmd* pcmd = cmd_list->CmdBuffer.begin(); pcmd != cmd_list->CmdBuffer.end(); pcmd++) {
      if (pcmd->UserCallback) {
          pcmd->UserCallback(cmd_list, pcmd);
      } else {
        GL(glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId));
        GL(glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y)));
        GL(glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, GL_UNSIGNED_SHORT, idx_buffer_offset));
      }
      idx_buffer_offset += pcmd->ElemCount;
    }
  }

  // Restore modified GL state
  glUseProgram(last_program);
  glBindTexture(GL_TEXTURE_2D, last_texture);
  glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, last_element_array_buffer);
  glBindVertexArray(last_vertex_array);
  glDisable(GL_SCISSOR_TEST);
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
  io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;                 // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
  io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
  io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
  io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
  io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
  io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
  io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
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
}

void CreateFontsTexture() {
  ImGuiIO &io = ImGui::GetIO();

  unsigned char* pixels;
  int width, height;
  io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bits for OpenGL3 demo because it is more likely to be compatible with user's existing shader.

  GL(glGenTextures(1, &imgui_manager_state.font_texture_handle));
  GL(glBindTexture(GL_TEXTURE_2D, imgui_manager_state.font_texture_handle));
  GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
  GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
  GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels));

  io.Fonts->TexID = (void *)(intptr_t)imgui_manager_state.font_texture_handle;

  io.Fonts->ClearInputData();
  io.Fonts->ClearTexData();
}

void CreateDeviceObjects() {
  // Backup GL state
  GLint last_texture, last_array_buffer, last_vertex_array;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
  glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);

  auto program_handle = imgui_manager_state.material_manager->CreateShaderFromSource("imgui_shader", kShaderSource);
  auto material = imgui_manager_state.material_manager->CreateMaterial(program_handle);
  imgui_manager_state.material = material;

  GL(imgui_manager_state.texture_location = glGetUniformLocation(material->program_handle(), "Texture"));

  imgui_manager_state.projection_uniform = material->Get<float, 4, 4>("ProjMtx");

  auto &allocator = foundation::memory_globals::default_allocator();
  imgui_manager_state.vbo = allocate_unique<BufferObject>(allocator, BufferObject::Target::Array);
  imgui_manager_state.ibo = allocate_unique<BufferObject>(allocator, BufferObject::Target::ElementArray);
  imgui_manager_state.vao = allocate_unique<ArrayObject>(allocator);

  auto &vao = *imgui_manager_state.vao;
  auto &vbo = *imgui_manager_state.vbo;

  using Im4Attribute = Attribute<ImVec4>;
  Im4Attribute color_attribute{2, Im4Attribute::DataType::UnsignedByte, Im4Attribute::DataOption::Normalized};

  vao.AddVertexBuffer(vbo, 0, Attribute<ImVec2>{0}, Attribute<ImVec2>{1}, color_attribute);

  CreateFontsTexture();

  // Restore modified GL state
  glBindTexture(GL_TEXTURE_2D, last_texture);
  glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
  glBindVertexArray(last_vertex_array);
}

void BeginFrame(double delta_time) {
  ImGuiIO &io = ImGui::GetIO();

  if (!imgui_manager_state.font_texture_handle) CreateDeviceObjects();

  io.DeltaTime = (float)delta_time;

  int w, h;
  int display_w, display_h;
  glfwGetWindowSize(imgui_manager_state.window, &w, &h);
  glfwGetFramebufferSize(imgui_manager_state.window, &display_w, &display_h);
  io.DisplaySize = ImVec2((float)w, (float)h);
  io.DisplayFramebufferScale = ImVec2((float)display_w / w, (float)display_h / h);

  if (glfwGetWindowAttrib(imgui_manager_state.window, GLFW_FOCUSED)) {
    double mouse_x, mouse_y;
    glfwGetCursorPos(imgui_manager_state.window, &mouse_x, &mouse_y);
    io.MousePos = ImVec2((float)mouse_x, (float)mouse_y);
  } else {
    io.MousePos = ImVec2(-1,-1);
  }

  for (int i = 0; i < 3; i++) {
    io.MouseDown[i] = imgui_manager_state.mouse_pressed[i] || glfwGetMouseButton(imgui_manager_state.window, i) != 0; // If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
    imgui_manager_state.mouse_pressed[i] = false;
  }

  io.MouseWheel = imgui_manager_state.mouse_wheel;
  imgui_manager_state.mouse_wheel = 0;

  glfwSetInputMode(imgui_manager_state.window, GLFW_CURSOR, io.MouseDrawCursor ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);

  ImGui::NewFrame();
}

void EndFrame() {
  ImGui::Render();
}

void OnMouse(int button, int action) {
  if (action == GLFW_PRESS && button >= 0 && button < 3) {
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

  io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
  io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
  io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
}

void OnCharacter(const unsigned int &character) {
  if (character > 0 && character < 0x10000) {
    ImGui::GetIO().AddInputCharacter((unsigned short)character);
  }
}

void OnScroll(const double &yoffset) {
  imgui_manager_state.mouse_wheel += (float)yoffset;
}

void Shutdown() {
  // TODO: Fix this by allocating imgui manager state and releasing here
  imgui_manager_state.material.reset();
  imgui_manager_state.vbo.reset();
  imgui_manager_state.ibo.reset();
  imgui_manager_state.vao.reset();
}

} // namespace ImGuiManager

} // namespace knight
