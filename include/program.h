#ifndef PROGRAM_H_
#define PROGRAM_H_

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
  DISALLOW_COPY_AND_ASSIGN(Program);
  GLuint handle_;
};

}; // namespace knight

#endif // PROGRAM_H_
