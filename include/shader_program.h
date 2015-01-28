#pragma once

#include "shader_types.h"

namespace knight {

namespace shader_program {
  foundation::Array<char> GetProgramInfoLog(GLuint program_handle);
  foundation::Array<char> GetShaderInfoLog(GLuint program_handle, GLuint shader_handle);
} // namespace shader_program

class ShaderProgram {
 public:
  ShaderProgram(foundation::Allocator &allocator);
  ~ShaderProgram();

  void Initialize(UniformFactory &uniform_factory, const char *source);

  GLuint handle() const { return handle_; }

  void Bind() const;
  void Unbind() const;
  
  GLint GetAttributeLocation(const GLchar *name);

  void PushUniforms();
  void NotifyDirty(const GLint &location, const UniformBase *uniform);

 private:
  GLuint handle_;
  GLuint vertex_handle_;
  GLuint fragment_handle_;
  foundation::Hash<const UniformBase *> dirty_uniforms_;

  KNIGHT_DISALLOW_COPY_AND_ASSIGN(ShaderProgram);
};

} // namespace knight
