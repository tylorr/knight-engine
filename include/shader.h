#ifndef SHADER_H_
#define SHADER_H_

#include "common.h"

#include <GL/glew.h>

#include <string>

namespace knight {

enum class ShaderType : GLenum {
  VERTEX = GL_VERTEX_SHADER,
  FRAGMENT = GL_FRAGMENT_SHADER,
  GEOMETRY = GL_GEOMETRY_SHADER
};

class Shader {
 public:
  Shader(ShaderType type);
  Shader(ShaderType type, const std::string &code);

  ~Shader();

  static std::string ReadSource(const char *filename);

  GLuint handle() const { return handle_; }

  void Source(const std::string &code);
  void Compile();

  std::string GetInfoLog() const;

 private:
  GLuint handle_;

  KNIGHT_DISALLOW_COPY_AND_ASSIGN(Shader);
};

}; // namespace knight

#endif // SHADER_H_
