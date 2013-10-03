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

#include "buffer_object.h"

GC BufferObject::gc;

BufferObject::BufferObject(GLenum target) 
    : target_(target) {
  BufferObject::gc.Create(handle_, glGenBuffers, glDeleteBuffers);
}

BufferObject::BufferObject(GLenum target, const GLsizeiptr &size, const GLvoid *data, const GLenum &usage)
    : target_(target) {
  BufferObject::gc.Create(handle_, glGenBuffers, glDeleteBuffers);
  Data(size, data, usage);
}

BufferObject::~BufferObject() {
  BufferObject::gc.Destroy(handle_);
}

void BufferObject::Data(const GLsizeiptr &size, const GLvoid *data, const GLenum &usage) {
  glBindBuffer(target_, handle_);
  glBufferData(target_, size, data, usage);
}

void BufferObject::SubData(const GLintptr &offset, const GLsizeiptr &size, const GLvoid *data) {
  glBindBuffer(target_, handle_);
  glBufferSubData(target_, offset, size, data);
}

void BufferObject::GetSubData(const GLintptr &offset, const GLsizeiptr &size, GLvoid *data) {
  glBindBuffer(target_, handle_);
  glGetBufferSubData(target_, offset, size, data);
}
