#include "uniform_factory.h"
#include "shader_program.h"

#include <logog.hpp>

#define KNIGHT_UNIFORM_GET_TYPE(TypeName, row_count, col_count, GLtype, type_string) \
  template<>                                                                         \
  struct UniformFactory::GetType<TypeName, row_count, col_count> {                   \
    static const GLenum value = GLtype;                                              \
    static const char *string;                                                       \
  };                                                                                 \
  const GLenum UniformFactory::GetType<TypeName, row_count, col_count>::value;       \
  const char *UniformFactory::GetType<TypeName, row_count, col_count>::string = type_string;

#define KNIGHT_UNIFORM_GET_VECTOR_TYPE(TypeName, count, GLtype) KNIGHT_UNIFORM_GET_TYPE(TypeName, count, 1, GLtype, #GLtype)
#define KNIGHT_UNIFORM_GET_MATRIX_TYPE(TypeName, count, GLtype) KNIGHT_UNIFORM_GET_TYPE(TypeName, count, count, GLtype, #GLtype)

namespace knight {

UniformBase *UniformFactory::Create(ShaderProgram *shader_program, const GLint &location, 
                    const char *name, const GLenum &type) {
  UniformBase *uniform;

  auto name_type_key = std::make_pair(name, type);
  auto it = uniforms_.find(name_type_key);
  if (it != uniforms_.end()) {
    uniform = (*it).second;
    uniform->AddShaderProgram(shader_program, location);
  } else {
    switch(type) {
      case GL_FLOAT_VEC4:
        uniform = new Uniform<float, 4>(name, shader_program, location);
    }

    uniforms_[name_type_key] = uniform;
  }

  return uniform;
}

KNIGHT_UNIFORM_GET_VECTOR_TYPE(float, 1, GL_FLOAT)
KNIGHT_UNIFORM_GET_VECTOR_TYPE(float, 2, GL_FLOAT_VEC2)
KNIGHT_UNIFORM_GET_VECTOR_TYPE(float, 3, GL_FLOAT_VEC3)
KNIGHT_UNIFORM_GET_VECTOR_TYPE(float, 4, GL_FLOAT_VEC4)

KNIGHT_UNIFORM_GET_VECTOR_TYPE(int, 1, GL_INT)
KNIGHT_UNIFORM_GET_VECTOR_TYPE(int, 2, GL_INT_VEC2)
KNIGHT_UNIFORM_GET_VECTOR_TYPE(int, 3, GL_INT_VEC3)
KNIGHT_UNIFORM_GET_VECTOR_TYPE(int, 4, GL_INT_VEC4)

KNIGHT_UNIFORM_GET_VECTOR_TYPE(bool, 1, GL_BOOL)
KNIGHT_UNIFORM_GET_VECTOR_TYPE(bool, 2, GL_BOOL_VEC2)
KNIGHT_UNIFORM_GET_VECTOR_TYPE(bool, 3, GL_BOOL_VEC3)
KNIGHT_UNIFORM_GET_VECTOR_TYPE(bool, 4, GL_BOOL_VEC4)

KNIGHT_UNIFORM_GET_MATRIX_TYPE(float, 2, GL_FLOAT_MAT2)
KNIGHT_UNIFORM_GET_MATRIX_TYPE(float, 3, GL_FLOAT_MAT3)
KNIGHT_UNIFORM_GET_MATRIX_TYPE(float, 4, GL_FLOAT_MAT4)

} // namespace knight
