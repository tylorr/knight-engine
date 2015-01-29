#pragma once

#include "shader_types.h"

namespace knight {

namespace shader_program {
  foundation::Array<char> GetProgramInfoLog(GLuint program_handle);
  foundation::Array<char> GetShaderInfoLog(GLuint program_handle, GLuint shader_handle);
} // namespace shader_program

} // namespace knight
