#ifndef PROGRAM_H_
#define PROGRAM_H_

#include "common.h"
#include "gc.h"
#include "shader.h"

#include <GL/glew.h>

#include <utility>
#include <queue>
#include <exception>
#include <string>

class Uniform;

class LinkException : public std::exception {
 public:
  LinkException(const std::string &str) throw() : info_log_(str) {}
  ~LinkException() throw() {}

  virtual const char *what() const throw() {
    return info_log_.c_str();
  }

 private:
  std::string info_log_;
};

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

  void NotifyDirty(Uniform *, const GLuint &);
  void UpdateProgram();

  GLint GetAttribute(const GLchar *name);
  
 private:
  typedef std::pair<Uniform *, GLuint> UniformLocPair;

  DISALLOW_COPY_AND_ASSIGN(Program);

  void UpdateUniform(Uniform *) const;

  static GC gc;

  GLuint handle_;
  std::queue<UniformLocPair> dirty_uniforms_;
};

#endif // PROGRAM_H_
