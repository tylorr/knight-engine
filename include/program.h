#ifndef KNIGHT_PROGRAM_H_
#define KNIGHT_PROGRAM_H_

#include "common.h"
#include "shader.h"

#include <GL/glew.h>

namespace knight {

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

  GLint GetAttribute(const GLchar *name);

 private:
  GLuint handle_;

  KNIGHT_DISALLOW_COPY_AND_ASSIGN(Program);
};

} // namespace knight

#endif // KNIGHT_PROGRAM_H_
