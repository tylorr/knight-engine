#include "uniform_manager.h"
#include "shader_program.h"
#include "uniform.h"
#include "iterators.h"

#include <logog.hpp>
#include <memory.h>
#include <murmur_hash.h>
#include <temp_allocator.h>

using namespace foundation;

#define KNIGHT_CREATE_UNIFORM_CASES(UpperTypeName, LowerTypeName)                                                               \
  case GL_ ## UpperTypeName:          uniform = allocator_.make_new<Uniform<LowerTypeName, 1>>(allocator_, *this, name); break; \
  case GL_ ## UpperTypeName ## _VEC2: uniform = allocator_.make_new<Uniform<LowerTypeName, 2>>(allocator_, *this, name); break; \
  case GL_ ## UpperTypeName ## _VEC3: uniform = allocator_.make_new<Uniform<LowerTypeName, 3>>(allocator_, *this, name); break; \
  case GL_ ## UpperTypeName ## _VEC4: uniform = allocator_.make_new<Uniform<LowerTypeName, 4>>(allocator_, *this, name); break;

namespace knight {

UniformManager::UniformManager(Allocator &allocator) 
    : allocator_{allocator},
      uniforms_{allocator},
      dirty_uniforms_{allocator} { }

UniformManager::~UniformManager() {
  for (auto &item : uniforms_) {
    allocator_.make_delete(item.value);
  }
  hash::clear(uniforms_);
}

UniformBase *UniformManager::Create(ShaderProgram &shader_program, 
                                    GLint location, 
                                    const char *name, 
                                    GLenum type) {
  auto program_handle = shader_program.handle();
  auto program_location_hash = uniform_manager::hash(program_handle, location);

  auto uniform = hash::get<UniformBase *>(uniforms_, program_location_hash, nullptr);

  if (uniform != nullptr) {
    uniform->AddShaderProgram(program_handle, location);
  } else {
    switch (type) {
      KNIGHT_CREATE_UNIFORM_CASES(FLOAT, float)
      KNIGHT_CREATE_UNIFORM_CASES(INT, int)
      KNIGHT_CREATE_UNIFORM_CASES(BOOL, bool)
      case GL_FLOAT_MAT2: uniform = allocator_.make_new<Uniform<float, 2, 2>>(allocator_, *this, name); break;
      case GL_FLOAT_MAT3: uniform = allocator_.make_new<Uniform<float, 3, 3>>(allocator_, *this, name); break;
      case GL_FLOAT_MAT4: uniform = allocator_.make_new<Uniform<float, 4, 4>>(allocator_, *this, name); break;
      // TODO: TR Handle samplers etc.
      // default: XASSERT(false, "Cannot make a uniform of this type 0x%X", type);
    }

    if (uniform != nullptr) {
      uniform->AddShaderProgram(program_handle, location);
      hash::set(uniforms_, program_location_hash, uniform);
    }
  }

  return uniform;
}

UniformBase *UniformManager::TryGet(const ShaderProgram &shader_program, GLint location) const {
  auto program_handle = shader_program.handle();
  auto program_location_hash = uniform_manager::hash(program_handle, location);
  return hash::get<UniformBase *>(uniforms_, program_location_hash, nullptr);
}

void UniformManager::NotifyDirty(GLuint program_handle, const UniformBase *uniform) {
  multi_hash::insert(dirty_uniforms_, program_handle, uniform);
}

void UniformManager::PushUniforms(const ShaderProgram &shader_program) {
  TempAllocator64 temp_allocator;
  auto dirty_unforms_for_program = Array<const UniformBase *>{temp_allocator};
  auto program_handle = shader_program.handle();
  multi_hash::get(dirty_uniforms_, program_handle, dirty_unforms_for_program);

  for (auto &uniform : dirty_unforms_for_program) {
    auto uniform_location = uniform->GetLocation(program_handle);
    uniform->Push(uniform_location);
  }
}

namespace uniform_manager {
  uint64_t hash(GLuint program_handle, GLint uniform_location) {
    return murmur_hash_64(&uniform_location, sizeof(uniform_location), program_handle);
  }
}

} // namespace knight
