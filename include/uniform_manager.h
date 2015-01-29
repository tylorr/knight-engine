#pragma once

#include "shader_types.h"

#include <logog.hpp>

namespace knight {

namespace uniform_manager {
  uint64_t hash(GLuint program_handle, GLint uniform_location);
}

template<typename T, size_t row_count, size_t col_count>
Uniform<T, row_count, col_count> *UniformManager::Get(
      const ShaderProgram &shader_program, 
      const char *name) const {

  auto location = shader_program.GetUniformLocation(name);
  auto uniform_base = TryGet(shader_program, location);

  XASSERT(uniform_base != nullptr, 
    "No active uniform with name '%s' in shader %u", name, shader_program.handle());

  return static_cast<Uniform<T, row_count, col_count> *>(uniform_base);
}

} // namespace knight
