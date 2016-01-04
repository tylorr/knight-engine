#pragma once

#include "shader_types.h"
#include "pointers.h"
#include "types.h"
#include "dependency_injection.h"

#include <glm/glm.hpp>

using namespace knight;

struct GLFWwindow;

struct GameState {
  std::shared_ptr<Material> material;
  pointer<BufferObject> vbo;
  pointer<BufferObject> ibo;

  GLFWwindow *window;

  pointer<ArrayObject> vao;

  Entity::ID entity_id;

  Uniform<float, 4, 4> *mvp_uniform;
  Uniform<float, 4, 4> *mv_matrix_uniform;
  Uniform<float, 3, 3> *normal_matrix_uniform;

  pointer<di::Injector> injector;

  char string_buff[256];
  char foo_buff[256];
};
