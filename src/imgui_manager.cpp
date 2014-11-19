#include "imgui_manager.h"

#include "shader.h"
#include "shader_program.h"
#include "uniform.h"
#include "buffer_object.h"
#include "vertex_array.h"
#include "bind.h"
#include "uniform_factory.h"

#include <GL/glew.h>
#include <stb_image.h>
#include <imgui.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace knight {

namespace ImGuiManager {
namespace {

const GLchar *kVertexSource =
  "#version 330\n"
  "uniform mat4 MVP;"
  "layout(location = 0) in vec2 i_pos;"
  "layout(location = 1) in vec2 i_uv;"
  "layout(location = 2) in vec4 i_col;"
  "out vec4 col;"
  "out vec2 pixel_pos;"
  "out vec2 uv;"
  "void main() {"
  "  col = i_col;"
  "  pixel_pos = i_pos;"
  "  uv = i_uv;"
  "  gl_Position = MVP * vec4(i_pos.x, i_pos.y, 0.0f, 1.0f);"
  "}";

const GLchar *kFragmentSource =
  "#version 330\n"
  "uniform sampler2D Tex;"
  "uniform vec4 ClipRect;"
  "in vec4 col;"
  "in vec2 pixel_pos;"
  "in vec2 uv;"
  "layout(location = 0) out vec4 o_col;"
  "void main() {"
  "  o_col = texture(Tex, uv) * col;"
  "  o_col.w *= (step(ClipRect.x,pixel_pos.x) * step(ClipRect.y,pixel_pos.y) * step(pixel_pos.x,ClipRect.z) * step(pixel_pos.y,ClipRect.w));"
  "}";

struct ImGuiManagerState {
  GLFWwindow *window;

  Shader vert_shader;
  Shader frag_shader;
  ShaderProgram shader_program;

  VertexArray vao;
  BufferObject vbo;

  GLuint font_texture_handle;

  Uniform<float, 4> *clip_rect_uniform;
};

ImGuiManagerState imgui_manager_state;

void RenderDrawLists(ImDrawList **const cmd_lists, int cmd_lists_count);

const char *GetClipboardString();
void SetClipboardString(const char *text, const char *text_end);

void RenderDrawLists(ImDrawList **const cmd_lists, int cmd_lists_count) {
  size_t total_vtx_count = 0;

  for (int n = 0; n < cmd_lists_count; n++) {
    total_vtx_count += cmd_lists[n]->vtx_buffer.size();
  }

  if (total_vtx_count == 0) {
    return;
  }

  const size_t vbo_size = total_vtx_count * sizeof(ImDrawVert);

  bind_guard<VertexArray> vao_bind(imgui_manager_state.vao);
  bind_guard<BufferObject> vbo_bind(imgui_manager_state.vbo);

  imgui_manager_state.vbo.Data(vbo_size, nullptr, GL_STREAM_DRAW);

  unsigned char *buffer_data = (unsigned char *)glMapBufferRange(
      GL_ARRAY_BUFFER, 0, vbo_size, 
      GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

  if (!buffer_data) {
    return;
  }

  for (int n = 0; n < cmd_lists_count; n++) {
    const ImDrawList* cmd_list = cmd_lists[n];
    memcpy(buffer_data, &cmd_list->vtx_buffer[0], cmd_list->vtx_buffer.size() * sizeof(ImDrawVert));
    buffer_data += cmd_list->vtx_buffer.size() * sizeof(ImDrawVert);
  }

  glUnmapBuffer(GL_ARRAY_BUFFER);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);

  bind_guard<ShaderProgram> program_bind(imgui_manager_state.shader_program);
  glBindTexture(GL_TEXTURE_2D, imgui_manager_state.font_texture_handle);

  int vtx_offset = 0;
  for (int n = 0; n < cmd_lists_count; n++)
  {
    const ImDrawList* cmd_list = cmd_lists[n];
    const ImDrawCmd* pcmd_end = cmd_list->commands.end();
    for (const ImDrawCmd* pcmd = cmd_list->commands.begin(); pcmd != pcmd_end; pcmd++)
    {
      imgui_manager_state.clip_rect_uniform->SetValue((float *)&pcmd->clip_rect);
      imgui_manager_state.shader_program.Update();

      glDrawArrays(GL_TRIANGLES, vtx_offset, pcmd->vtx_count);
      vtx_offset += pcmd->vtx_count;
    }
  }

  // Cleanup GL state
  glBindTexture(GL_TEXTURE_2D, 0);
}

const char *GetClipboardString() {
  return glfwGetClipboardString(imgui_manager_state.window);
}

void SetClipboardString(const char *text, const char *text_end) {
  if (!text_end) {
    text_end = text + strlen(text);
  }

  if (*text_end == '\0') {
    glfwSetClipboardString(imgui_manager_state.window, text);
  } else {
    // String needs to be null terminated so thatt glfw knows how long it is
    size_t text_length = text_end - text;

    char *buf = (char *)malloc(text_length + 1);
    memcpy(buf, text, text_length);
    buf[text_end-text] = '\0';

    glfwSetClipboardString(imgui_manager_state.window, buf);

    free(buf);
  }
}

} // namespace

void Initialize(GLFWwindow *window, UniformFactory *uniform_factory) {
  imgui_manager_state.window = window;

  int width, height;
  glfwGetWindowSize(imgui_manager_state.window, &width, &height);

  ImGuiIO &io = ImGui::GetIO();

  io.DisplaySize = ImVec2((float)width, (float)height);
  io.DeltaTime = 1.0f/60.0f;
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

  glGenTextures(1, &imgui_manager_state.font_texture_handle);
  glBindTexture(GL_TEXTURE_2D, imgui_manager_state.font_texture_handle);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  const void *png_data;
  unsigned int png_size;
  ImGui::GetDefaultFontData(nullptr, nullptr, &png_data, &png_size);

  int tex_x, tex_y;
  void *tex_data = stbi_load_from_memory((const unsigned char *)png_data, 
                                         (int)png_size, 
                                         &tex_x, &tex_y, 
                                         nullptr, 0);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_x, tex_y, 
               0, GL_RGBA, GL_UNSIGNED_BYTE, tex_data);
  stbi_image_free(tex_data);
  glBindTexture(GL_TEXTURE_2D, 0);

  imgui_manager_state.vert_shader.Initialize(ShaderType::VERTEX, kVertexSource);
  imgui_manager_state.frag_shader.Initialize(ShaderType::FRAGMENT, kFragmentSource);
  imgui_manager_state.shader_program.Initialize(imgui_manager_state.vert_shader, 
                                                imgui_manager_state.frag_shader, 
                                                *uniform_factory);

  glm::mat4 mvp = glm::ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, +1.0f);

  auto mvp_uniform = uniform_factory->Get<float, 4, 4>("MVP");
  mvp_uniform->SetValue(glm::value_ptr(mvp));

  imgui_manager_state.clip_rect_uniform = uniform_factory->Get<float, 4>("ClipRect");
  
  imgui_manager_state.vbo.Initialize(GL_ARRAY_BUFFER);
  imgui_manager_state.vao.Initialize();

  bind_guard<VertexArray> vao_bind(imgui_manager_state.vao);
  bind_guard<BufferObject> vbo_bind(imgui_manager_state.vbo);

  imgui_manager_state.vao.BindAttribute(0, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (const GLvoid *)0);
  imgui_manager_state.vao.BindAttribute(1, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (const GLvoid *)(2 * sizeof(float)));
  imgui_manager_state.vao.BindAttribute(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (const GLvoid *)(4 * sizeof(float)));
}

void BeginFrame() {
  ImGuiIO &io = ImGui::GetIO();

  static double previous_time = 0.0f;
  const double current_time =  glfwGetTime();
  io.DeltaTime = (float)(current_time - previous_time);
  previous_time = current_time;

  double mouse_x, mouse_y;
  glfwGetCursorPos(imgui_manager_state.window, &mouse_x, &mouse_y);
  io.MousePos = ImVec2((float)mouse_x, (float)mouse_y);
  io.MouseDown[0] = glfwGetMouseButton(imgui_manager_state.window, GLFW_MOUSE_BUTTON_LEFT) != 0;
  io.MouseDown[1] = glfwGetMouseButton(imgui_manager_state.window, GLFW_MOUSE_BUTTON_RIGHT) != 0;

  ImGui::NewFrame();
}

void EndFrame() {
  ImGui::Render();

  ImGuiIO &io = ImGui::GetIO();
  io.MouseWheel = 0;
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
    ImGui::GetIO().AddInputCharacter((char)character);
  }
}

void OnScroll(const double &yoffset) {
  ImGuiIO &io = ImGui::GetIO();
  io.MouseWheel = (yoffset != 0.0f) ? (yoffset > 0.0f ? 1 : - 1) : 0;
}

} // namespace ImGuiManager

} // namespace knight
