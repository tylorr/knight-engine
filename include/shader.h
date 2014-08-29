#ifndef KNIGHT_SHADER_H_
#define KNIGHT_SHADER_H_

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
  Shader(const ShaderType &type) 
    : handle_(glCreateShader(static_cast<GLenum>(type))) { }
  ~Shader();

  void Initialize(const std::string &code);

  GLuint handle() const { return handle_; }

  std::string GetInfoLog() const;

 private:
  GLuint handle_;

  void Source(const std::string &code);
  void Compile();

  KNIGHT_DISALLOW_COPY_AND_ASSIGN(Shader);
};

} // namespace knight

#endif // KNIGHT_SHADER_H_
