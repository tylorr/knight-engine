#pragma once

#include "game_memory.h"
#include "shader_types.h"
#include "pointers.h"
#include "types.h"
#include "dependency_injection.h"

#include <glm/glm.hpp>

struct GameState : public knight::GameAllocatorState {
  std::shared_ptr<knight::Material> material;
  knight::pointer<knight::BufferObject> vbo;
  knight::pointer<knight::BufferObject> ibo;

  knight::VertexArray vao;

  glm::mat4 model_matrix;
  knight::Uniform<float, 4, 4> *mvp_uniform;
  knight::Uniform<float, 4, 4> *mv_matrix_uniform;
  knight::Uniform<float, 3, 3> *normal_matrix_uniform;

  knight::pointer<knight::di::Injector> injector;

  uint32_t index_count;

  char string_buff[256];
  char foo_buff[256];
};
