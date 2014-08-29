#ifndef KNIGHT_SHADER_PROGRAM_H_
#define KNIGHT_SHADER_PROGRAM_H_

#include "common.h"
#include "shader.h"

#include <GL/glew.h>

#include <map>

namespace knight {

class UniformFactory;
class UniformBase;

class ShaderProgram {
 public:
  ShaderProgram();
  ShaderProgram(UniformFactory *uniform_factory, const Shader &vertex, 
                const Shader &fragment);
  ~ShaderProgram();

  GLuint handle() const { return handle_; }

  void Attach(const Shader &shader);
  void Link();
  void Bind() const;

  std::string GetInfoLog() const;

  GLint GetAttribute(const GLchar *name);

  void Update();

  void NotifyDirty(const GLint &location, const UniformBase *uniform);

  void ExtractUniforms();

 private:
  UniformFactory *uniform_factory_;
  GLuint handle_;

  std::map<GLint, const UniformBase *> dirty_uniforms_;

  KNIGHT_DISALLOW_COPY_AND_ASSIGN(ShaderProgram);
};

} // namespace knight

#endif // KNIGHT_SHADER_PROGRAM_H_
