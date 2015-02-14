#pragma once

#include "shader_types.h"
#include "iterators.h"

#include <hash.h>
#include <logog.hpp>

namespace knight {

class Material {
 public:
  // Material(foundation::Allocator &alloc) 
  //     : program_handle_{0},
  //       version_{0},
  //       uniforms_{alloc} { }

  Material(foundation::Allocator &alloc, GLuint program_handle, uint32_t version, 
           foundation::Array<UniformBase *> uniform_list);

  GLuint program_handle() const { return program_handle_; }
  uint32_t version() const { return version_; }
  //TODO: TR Implement GetUniform methods here

  void Bind() const {
    //GL(glUseProgram(program_handle_));
    glUseProgram(program_handle_);
    GL_ASSERT("Trying to bind material program: %u", program_handle_);
  }
  void Unbind() const {
    auto current_program = GLint{};
    glGetIntegerv(GL_CURRENT_PROGRAM, &current_program);

    if (program_handle_ == current_program) {
      GL(glUseProgram(0));
    }
  }

  template<typename T, size_t row_count, size_t col_count = 1>
  Uniform<T, row_count, col_count> *Get(const char *name) const;

  template<typename T, size_t row_count, size_t col_count = 1>
  Uniform<T, row_count, col_count> *Get(GLint location) const;

 private:
  GLuint program_handle_;
  uint32_t version_;
  foundation::Hash<UniformBase *> uniforms_;
};


class MaterialManager {
 public:
  MaterialManager(foundation::Allocator &alloc);
  MaterialManager(foundation::Allocator &alloc, foundation::Array<const char *> global_uniforms);
  ~MaterialManager();

  GLuint CreateShader(const char *shader_path);
  GLuint CreateShaderFromSource(const char *name, const char *shader_source);
  GLuint CreateShaderFromSource(uint64_t hash, const char *shader_source);

  std::shared_ptr<Material> CreateMaterial(const char *shader_path);
  std::shared_ptr<Material> CreateMaterial(GLuint program_handle);
  std::shared_ptr<Material> CloneMaterial(const std::shared_ptr<Material> &other);

  void MarkDirty(UniformBase *uniform, const Material &mat, GLint location);
  void PushUniforms(const Material &mat);

 private:
  struct ShaderHandles {
    GLuint program;
    GLuint vertex;
    GLuint fragment;
  };

  struct DirtyUniform {
    UniformBase *uniform;
    GLint location;
  };

  foundation::Allocator &alloc_;
  const foundation::Array<const char *> global_uniforms_;
  foundation::Hash<ShaderHandles> shaders_;
  foundation::Hash<uint32_t> material_version_;
  foundation::Hash<UniformBase *> uniforms_;
  foundation::Hash<DirtyUniform> dirty_uniforms_;
  
  KNIGHT_DISALLOW_COPY_AND_ASSIGN(MaterialManager);
};



template<typename T, size_t row_count, size_t col_count>
Uniform<T, row_count, col_count> *Material::Get(const char *name) const {
  auto location = GLint{};
  GL(location = glGetUniformLocation(program_handle_, name));
  auto uniform_base = foundation::hash::get<UniformBase *>(uniforms_, location, nullptr);

  XASSERT(uniform_base != nullptr, 
    "No active uniform with name '%s' in shader %u", name, program_handle_);

  return dynamic_cast<Uniform<T, row_count, col_count> *>(uniform_base);
}

template<typename T, size_t row_count, size_t col_count>
Uniform<T, row_count, col_count> *Material::Get(GLint location) const {
  auto uniform_base = foundation::hash::get<UniformBase *>(uniforms_, location, nullptr);

  XASSERT(uniform_base != nullptr, 
    "No active uniform at location %d in shader %u", location, program_handle_);

  return dynamic_cast<Uniform<T, row_count, col_count> *>(uniform_base);
}
  
} // namespace knight
