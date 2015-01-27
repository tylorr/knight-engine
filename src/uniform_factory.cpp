#include "uniform_factory.h"
#include "shader_program.h"
#include "uniform.h"

#include <logog.hpp>
#include <hash.h>

#define KNIGHT_GET_UNIFORM_TYPE(TypeName, row_count, col_count, GLtype, type_string)         \
  template<>                                                                                 \
  struct UniformFactory::GetType<TypeName, row_count, col_count> {                           \
    static const GLenum value = GLtype;                                                      \
    static const char *name;                                                                 \
  };                                                                                         \
  const GLenum UniformFactory::GetType<TypeName, row_count, col_count>::value;               \
  const char *UniformFactory::GetType<TypeName, row_count, col_count>::name = type_string;

#define KNIGHT_GET_UNIFORM_VECTOR_TYPE(TypeName, count, GLtype) KNIGHT_GET_UNIFORM_TYPE(TypeName, count, 1, GLtype, #GLtype)
#define KNIGHT_GET_UNIFORM_MATRIX_TYPE(TypeName, count, GLtype) KNIGHT_GET_UNIFORM_TYPE(TypeName, count, count, GLtype, #GLtype)

#define KNIGHT_CREATE_UNIFORM_CASES(UpperTypeName, LowerTypeName)                                            \
  case GL_ ## UpperTypeName:          uniform = allocator_.make_new<Uniform<LowerTypeName, 1>>(name); break; \
  case GL_ ## UpperTypeName ## _VEC2: uniform = allocator_.make_new<Uniform<LowerTypeName, 2>>(name); break; \
  case GL_ ## UpperTypeName ## _VEC3: uniform = allocator_.make_new<Uniform<LowerTypeName, 3>>(name); break; \
  case GL_ ## UpperTypeName ## _VEC4: uniform = allocator_.make_new<Uniform<LowerTypeName, 4>>(name); break;

namespace knight {

UniformFactory::UniformFactory(foundation::Allocator &allocator) 
    : uniforms_(allocator), 
      allocator_(allocator) { }

UniformFactory::~UniformFactory() {
  auto it = foundation::hash::begin(uniforms_);
  auto end = foundation::hash::end(uniforms_);
  for (; it != end; ++it) {
    allocator_.make_delete(it->value);
  }
  foundation::hash::clear(uniforms_);
}

UniformBase *UniformFactory::Create(ShaderProgram &shader_program, 
                                    const GLint &location, 
                                    const char *name, 
                                    const GLenum &type) {
  auto key = uniform::hash(name, type);
  auto uniform = foundation::hash::get<UniformBase *>(uniforms_, key, nullptr);

  if (uniform != nullptr) {
    uniform->AddShaderProgram(shader_program, location);
  } else {
    switch (type) {
      KNIGHT_CREATE_UNIFORM_CASES(FLOAT, float)
      KNIGHT_CREATE_UNIFORM_CASES(INT, int)
      KNIGHT_CREATE_UNIFORM_CASES(BOOL, bool)
      case GL_FLOAT_MAT2: uniform = allocator_.make_new<Uniform<float, 2, 2>>(name); break;
      case GL_FLOAT_MAT3: uniform = allocator_.make_new<Uniform<float, 3, 3>>(name); break;
      case GL_FLOAT_MAT4: uniform = allocator_.make_new<Uniform<float, 4, 4>>(name); break;
      // TODO: TR Handle samplers etc.
      // default: XASSERT(false, "Cannot make a uniform of this type 0x%X", type);
    }

    if (uniform != nullptr) {
      uniform->AddShaderProgram(shader_program, location);
      foundation::hash::set(uniforms_, key, uniform);
    }
  }

  return uniform;
}

UniformBase *UniformFactory::TryGet(const char *name, GLenum type) const {
  auto key = uniform::hash(name, type);
  return foundation::hash::get<UniformBase *>(uniforms_, key, nullptr);
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
