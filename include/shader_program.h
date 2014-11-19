#pragma once

#include "common.h"

#include <GL/glew.h>

#include <map>

namespace knight {

class UniformFactory;
class UniformBase;

class ShaderProgram {
 public:
  ShaderProgram() 
    : handle_{}, 
      vertex_handle_{}, 
      fragment_handle_{}, 
      dirty_uniforms_{} { }

  ShaderProgram(ShaderProgram &&other)
    : handle_(std::move(other.handle_)),
      vertex_handle_(std::move(other.vertex_handle_)),
      fragment_handle_(std::move(other.fragment_handle_)),
      dirty_uniforms_(std::move(other.dirty_uniforms_)) {
    other.handle_ = 0;
    other.vertex_handle_ = 0;
    other.fragment_handle_ = 0;
    other.dirty_uniforms_.clear(); 
  }

  ~ShaderProgram();

  void Initialize(UniformFactory &uniform_factory, const std::string &source);

  GLuint handle() const { return handle_; }

  void Bind() const;
  void Unbind() const;
  
  GLint GetAttributeLocation(const GLchar *name);

  void Update();
  void NotifyDirty(const GLint &location, const UniformBase *uniform);

  ShaderProgram &operator=(ShaderProgram &&other) {
    handle_ = std::move(other.handle_);
    dirty_uniforms_ = std::move(other.dirty_uniforms_);
    return *this;
  }

 private:
  GLuint handle_;
  GLuint vertex_handle_;
  GLuint fragment_handle_;
  std::map<GLint, const UniformBase *> dirty_uniforms_;

  GLuint CreateAndAttachShader(GLenum type, const std::string &source);
  void Link();
  void ExtractUniforms(UniformFactory &uniform_factory);

  std::string GetProgramInfoLog() const;
  std::string GetShaderInfoLog(GLuint shader_handle) const;

  KNIGHT_DISALLOW_COPY_AND_ASSIGN(ShaderProgram);
};

} // namespace knight
