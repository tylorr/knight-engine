#pragma once

#include "shader_types.h"
#include "gl_util.h"
#include "iterators.h"
#include "vector.h"

#include <hash.h>
#include <logog.hpp>
#include <gsl.h>

namespace knight {

class Material {
 public:
  // Material(foundation::Allocator &alloc)
  //     : program_handle_{0},
  //       version_{0},
  //       uniforms_{alloc} { }

  Material(foundation::Allocator &alloc, GLuint program_handle, uint32_t version,
           Vector<UniformBase *> uniform_list);

  GLuint program_handle() const { return program_handle_; }
  uint32_t version() const { return version_; }
  //TODO: TR Implement GetUniform methods here

  void bind() const {
    //GL(glUseProgram(program_handle_));
    glUseProgram(program_handle_);
    GL_ASSERT("Trying to bind material program: %u", program_handle_);
  }
  void unbind() const {
    auto current_program = GLint{};
    glGetIntegerv(GL_CURRENT_PROGRAM, &current_program);

    if (program_handle_ == (GLuint)current_program) {
      GL(glUseProgram(0));
    }
  }

  template<typename T, size_t row_count, size_t col_count = 1>
  Uniform<T, row_count, col_count> *get(gsl::czstring<> name) const;

  template<typename T, size_t row_count, size_t col_count = 1>
  Uniform<T, row_count, col_count> *get(GLint location) const;

 private:
  GLuint program_handle_;
  uint32_t version_;
  foundation::Hash<UniformBase *> uniforms_;
};

bool operator==(const Material &a, const Material &b);
bool operator!=(const Material &a, const Material &b);
bool operator<(const Material &a, const Material &b);
bool operator>(const Material &a, const Material &b);


class MaterialManager {
 public:
  MaterialManager(foundation::Allocator &alloc);
  MaterialManager(foundation::Allocator &alloc, Vector<gsl::czstring<>> global_uniforms);
  ~MaterialManager();

  GLuint create_shader(gsl::czstring<> shader_path);
  GLuint create_shader_from_source(gsl::czstring<> name, gsl::czstring<> shader_source);
  GLuint create_shader_from_source(uint64_t hash, gsl::czstring<> shader_source);

  std::shared_ptr<Material> create_material(gsl::czstring<> shader_path);
  std::shared_ptr<Material> create_material(GLuint program_handle);
  std::shared_ptr<Material> clone_material(const std::shared_ptr<Material> &other);

  void mark_dirty(UniformBase *uniform, const Material &mat, GLint location);
  void push_uniforms(const Material &mat);

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
  const Vector<gsl::czstring<>> global_uniforms_;
  foundation::Hash<ShaderHandles> shaders_;
  foundation::Hash<uint32_t> material_version_;
  foundation::Hash<UniformBase *> uniforms_;
  foundation::Hash<DirtyUniform> dirty_uniforms_;

  OpenglVersion opengl_version_;

  KNIGHT_DISALLOW_COPY_AND_ASSIGN(MaterialManager);
};

template<typename T, size_t row_count, size_t col_count>
Uniform<T, row_count, col_count> *Material::get(gsl::czstring<> name) const {
  auto location = GLint{};
  GL(location = glGetUniformLocation(program_handle_, name));
  auto uniform_base = foundation::hash::get<UniformBase *>(uniforms_, location, nullptr);

  XASSERT(uniform_base != nullptr,
    "No active uniform with name '%s' in shader %u", name, program_handle_);

  return dynamic_cast<Uniform<T, row_count, col_count> *>(uniform_base);
}

template<typename T, size_t row_count, size_t col_count>
Uniform<T, row_count, col_count> *Material::get(GLint location) const {
  auto uniform_base = foundation::hash::get<UniformBase *>(uniforms_, location, nullptr);

  XASSERT(uniform_base != nullptr,
    "No active uniform at location %d in shader %u", location, program_handle_);

  return dynamic_cast<Uniform<T, row_count, col_count> *>(uniform_base);
}

} // namespace knight
