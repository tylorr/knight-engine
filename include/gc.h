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

#ifndef GC_H_
#define GC_H_

#include <map>

#include <GL/glew.h>

/*
  OpenGL object creation/destruction function prototypes
*/

#if defined( _WIN32 )
typedef void (__stdcall *CreateFunc) (GLsizei, GLuint *);
typedef void (__stdcall *DeleteFunc) (GLsizei, const GLuint *);
typedef void (__stdcall *DeleteFunc2) (GLuint);
#else
typedef void (*CreateFunc) (GLsizei, GLuint *);
typedef void (*DeleteFunc) (GLsizei, const GLuint *);
typedef void (*DeleteFunc2) (GLuint);
#endif

/*
  OpenGL object garbage collector
*/
class GC {
 public:
  void Create(GLuint &obj, CreateFunc createFunc, DeleteFunc deleteFunc) {
    createFunc(1, &obj);
    refs_.insert( std::pair<GLuint, unsigned int>( obj, 1 ) );

    delete_func_ = deleteFunc;
    delete_func_2_ = nullptr;
  }

  int Create(const GLuint& obj, DeleteFunc2 deleteFunc2) {
    refs_.insert(std::pair<GLuint, unsigned int>(obj, 1));

    delete_func_ = nullptr;
    delete_func_2_ = deleteFunc2;

    return obj;
  }

  void Copy(const GLuint &from, GLuint &to, bool destructive = false) {
    if (destructive) {
      Destroy(to);
    }

    to = from;
    refs_[from]++;
  }

  void Destroy(GLuint &obj) {
    if (--refs_[obj] == 0) {
      if (delete_func_) {
        delete_func_(1, &obj);
      } else {
        delete_func_2_(obj);
      }
      refs_.erase(obj);
    }
  }

 private:
  std::map<GLuint, unsigned int> refs_;
  DeleteFunc delete_func_;
  DeleteFunc2 delete_func_2_;
};

#endif // GL_H_
