#pragma once

#include "common.h"

#include <collection_types.h>
#include <memory_types.h>

#include <GL/glew.h>

namespace knight {

class ShaderProgram;
class UniformBase;

template<typename T, size_t row_count, size_t col_count>
class Uniform;

class UniformManager;

class ShaderProgram {
 public:
  ShaderProgram() 
    : handle_{0}, 
      vertex_handle_{0}, 
      fragment_handle_{0} { }
  ~ShaderProgram();

  void Initialize(UniformManager &uniform_manager, const char *source);

  GLuint handle() const { return handle_; }

  void Bind() const;
  void Unbind() const;
  
  GLint GetUniformLocation(const GLchar *name) const;
  GLint GetAttributeLocation(const GLchar *name) const;

  GLuint handle_;
  GLuint vertex_handle_;
  GLuint fragment_handle_;
  
 private:
  KNIGHT_DISALLOW_COPY_AND_ASSIGN(ShaderProgram);
};

class UniformManager {
 public:
  UniformManager(foundation::Allocator &allocator);
  ~UniformManager();

  UniformBase *Create(ShaderProgram &shader_program, GLint location, 
                      const char *name, GLenum type);

  template<typename T, size_t row_count, size_t col_count = 1>
  Uniform<T, row_count, col_count> *Get(const ShaderProgram &shader_program, const char *name) const;
  UniformBase *TryGet(const ShaderProgram &shader_program, GLint location) const;

  void NotifyDirty(GLuint program_handle, const UniformBase *uniform);

  void PushUniforms(const ShaderProgram &shader_program);

  foundation::Allocator &allocator_;
  foundation::Hash<UniformBase *> uniforms_;
  foundation::Hash<const UniformBase *> dirty_uniforms_;

 private:
  KNIGHT_DISALLOW_COPY_AND_ASSIGN(UniformManager);
  KNIGHT_DISALLOW_MOVE_AND_ASSIGN(UniformManager);
};

} // namespace knight