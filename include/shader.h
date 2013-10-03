/*
  Copyright (C) 2012 Alexander Overvoorde

  Permission is hereby granted, free of charge, to any person obtaining a copy of
  this software and associated documentation files (the "Software"), to deal in
  the Software without restriction, including without limitation the rights to
  use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
  the Software, and to permit persons to whom the Software is furnished to do so,
  subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
  FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE
*/

#ifndef SHADER_H_
#define SHADER_H_

#include "gc.h"
#include "common.h"

#include <GL/glew.h>
  
#include <exception>
#include <string>

/*
  Shader type
*/
namespace ShaderType
{
  enum shader_type_t 
  {
    Vertex = GL_VERTEX_SHADER,
    Fragment = GL_FRAGMENT_SHADER,
    Geometry = GL_GEOMETRY_SHADER
  };
}

/*
  Exceptions
*/
class CompileException : public std::exception {
 public:
  CompileException(const std::string& str) throw() : info_log_(str) {}
  ~CompileException() throw() {}

  virtual const char* what() const throw() {
    return info_log_.c_str();
  }

 private:
  std::string info_log_;
};

/*
  Shader
*/
class Shader {
 public:
  // Shader(const Shader &other);
  Shader(ShaderType::shader_type_t type);
  Shader(ShaderType::shader_type_t type, const std::string &code);

  ~Shader();

  // operator GLuint() const;
  // const Shader &operator=(const Shader &other);

  GLuint handle() const { return handle_; }

  void Source(const std::string &code);
  void Compile();

  std::string GetInfoLog();

 private:
  DISALLOW_COPY_AND_ASSIGN(Shader);

  static GC gc;
  GLuint handle_;
};

#endif // SHADER_H_
