#ifndef PROGRAM_H_
#define PROGRAM_H_

#include "common.h"
#include "shader.h"

#include <GL/glew.h>

namespace knight {

// class Uniform;

class Program {
 public:
  Program();
  Program(const Shader &vertex, const Shader &fragment);
  ~Program();

  GLuint handle() const { return handle_; }

  void Attach(const Shader &shader);
  void Link();
  void Bind() const;

  std::string GetInfoLog() const;

  void ExtractShaderUniforms();

  // void NotifyDirty(Uniform *, const GLuint &);
  // void UpdateProgram();

  GLint GetAttribute(const GLchar *name);

 private:
  // typedef std::pair<Uniform *, GLuint> UniformLocPair;

  DISALLOW_COPY_AND_ASSIGN(Program);

  // void UpdateUniform(Uniform *) const;

  GLuint handle_;
  // std::queue<UniformLocPair> dirty_uniforms_;
};

}; // namespace knight

#endif // PROGRAM_H_
