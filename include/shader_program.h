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
  ShaderProgram() : handle_(glCreateProgram()) { }
  ~ShaderProgram();

  void Initialize(const Shader &vertex, const Shader &fragment,
                  UniformFactory *uniform_factory);

  GLuint handle() const { return handle_; }

  void Bind() const;

  std::string GetInfoLog() const;
  GLint GetAttributeLocation(const GLchar *name);

  void Update();
  void NotifyDirty(const GLint &location, const UniformBase *uniform);

 private:
  GLuint handle_;
  std::map<GLint, const UniformBase *> dirty_uniforms_;

  void Attach(const Shader &shader);
  void Link();
  void ExtractUniforms(UniformFactory *uniform_factory);

  KNIGHT_DISALLOW_COPY_AND_ASSIGN(ShaderProgram);
};

} // namespace knight

#endif // KNIGHT_SHADER_PROGRAM_H_
