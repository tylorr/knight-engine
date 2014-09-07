#include "uniform_factory.h"
#include "shader_program.h"

#include <logog.hpp>

#define KNIGHT_GET_UNIFORM_TYPE(TypeName, row_count, col_count, GLtype, type_string)             \
  template<>                                                                                 \
  struct UniformFactory::GetType<TypeName, row_count, col_count> {                           \
    static const GLenum value = GLtype;                                                      \
    static const char *string;                                                               \
  };                                                                                         \
  const GLenum UniformFactory::GetType<TypeName, row_count, col_count>::value;               \
  const char *UniformFactory::GetType<TypeName, row_count, col_count>::string = type_string;

#define KNIGHT_GET_UNIFORM_VECTOR_TYPE(TypeName, count, GLtype) KNIGHT_GET_UNIFORM_TYPE(TypeName, count, 1, GLtype, #GLtype)
#define KNIGHT_GET_UNIFORM_MATRIX_TYPE(TypeName, count, GLtype) KNIGHT_GET_UNIFORM_TYPE(TypeName, count, count, GLtype, #GLtype)

namespace knight {

UniformFactory::UniformFactory() {
  Register<float, 1>(GL_FLOAT);
  Register<float, 2>(GL_FLOAT_VEC2);
  Register<float, 3>(GL_FLOAT_VEC3);
  Register<float, 4>(GL_FLOAT_VEC4);

  Register<int, 1>(GL_INT);
  Register<int, 2>(GL_INT_VEC2);
  Register<int, 3>(GL_INT_VEC3);
  Register<int, 4>(GL_INT_VEC4);

  Register<bool, 1>(GL_BOOL);
  Register<bool, 2>(GL_BOOL_VEC2);
  Register<bool, 3>(GL_BOOL_VEC3);
  Register<bool, 4>(GL_BOOL_VEC4);

  Register<float, 2, 2>(GL_FLOAT_MAT2);
  Register<float, 3, 3>(GL_FLOAT_MAT3);
  Register<float, 4, 4>(GL_FLOAT_MAT4);
}

UniformFactory::~UniformFactory() {
  for (auto it = uniforms_.begin(); it != uniforms_.end(); ++it) {
    delete it->second;
  }
}

UniformBase *UniformFactory::Create(ShaderProgram *shader_program, 
                                    const GLint &location, 
                                    const char *name, 
                                    const GLenum &type) {
  UniformBase *uniform = nullptr;

  auto name_type_key = std::make_pair(name, type);
  auto it = uniforms_.find(name_type_key);

  if (it != uniforms_.end()) {
    uniform = (*it).second;
    uniform->AddShaderProgram(shader_program, location);
  } else {
    auto factory_it = uniform_factories_.find(type);

    if (factory_it != uniform_factories_.end()) {
      auto uniform_factory = (*factory_it).second;
      uniform = uniform_factory(name);
      uniform->AddShaderProgram(shader_program, location);
      
      uniforms_[name_type_key] = uniform;
    }
  }

  return uniform;
}

KNIGHT_GET_UNIFORM_VECTOR_TYPE(float, 1, GL_FLOAT)
KNIGHT_GET_UNIFORM_VECTOR_TYPE(float, 2, GL_FLOAT_VEC2)
KNIGHT_GET_UNIFORM_VECTOR_TYPE(float, 3, GL_FLOAT_VEC3)
KNIGHT_GET_UNIFORM_VECTOR_TYPE(float, 4, GL_FLOAT_VEC4)

KNIGHT_GET_UNIFORM_VECTOR_TYPE(int, 1, GL_INT)
KNIGHT_GET_UNIFORM_VECTOR_TYPE(int, 2, GL_INT_VEC2)
KNIGHT_GET_UNIFORM_VECTOR_TYPE(int, 3, GL_INT_VEC3)
KNIGHT_GET_UNIFORM_VECTOR_TYPE(int, 4, GL_INT_VEC4)

KNIGHT_GET_UNIFORM_VECTOR_TYPE(bool, 1, GL_BOOL)
KNIGHT_GET_UNIFORM_VECTOR_TYPE(bool, 2, GL_BOOL_VEC2)
KNIGHT_GET_UNIFORM_VECTOR_TYPE(bool, 3, GL_BOOL_VEC3)
KNIGHT_GET_UNIFORM_VECTOR_TYPE(bool, 4, GL_BOOL_VEC4)

KNIGHT_GET_UNIFORM_MATRIX_TYPE(float, 2, GL_FLOAT_MAT2)
KNIGHT_GET_UNIFORM_MATRIX_TYPE(float, 3, GL_FLOAT_MAT3)
KNIGHT_GET_UNIFORM_MATRIX_TYPE(float, 4, GL_FLOAT_MAT4)

} // namespace knight
