#pragma once

#include "common.h"

#include <collection_types.h>
#include <memory_types.h>

#include <GL/glew.h>

namespace knight {

class UniformBase;
template<typename T, size_t row_count, size_t col_count>
class Uniform;

class MaterialManager;
class Material;

struct BufferObject {
  BufferObject() { }
  BufferObject(BufferObject &&other)
    : handle_(std::move(other.handle_)),
      target_(std::move(other.target_)) { }

  ~BufferObject();

  GLuint handle() const { return handle_; }
  GLenum target() const { return target_; }

  void Initialize(const GLenum &target);
  void Initialize(const GLenum &target, const GLsizeiptr &size, 
                  const GLvoid *data, const GLenum &usage);

  void Bind() const;
  void Unbind() const;

  void Data(const GLsizeiptr &size, const GLvoid *data, const GLenum &usage);
  void SubData(const GLintptr &offset, const GLsizeiptr &size, const GLvoid *data);

  void GetSubData(const GLintptr &offset, const GLsizeiptr &size, GLvoid *data);

  BufferObject &operator=(BufferObject &&other) {
    handle_ = std::move(other.handle_);
    target_ = std::move(other.target_);
    return *this;
  }

  GLuint handle_;
  GLenum target_;

 private:
  KNIGHT_DISALLOW_COPY_AND_ASSIGN(BufferObject);
};

struct VertexArray {
  VertexArray() { }
  VertexArray(VertexArray &&other) : handle_{std::move(other.handle_)} { }
  ~VertexArray();

  GLuint handle() const { return handle_; }

  void Initialize();

  void Bind() const;
  void Unbind() const;

  void BindAttribute(const GLint& attribute,
                     const GLint &size, const GLenum &type,
                     const GLboolean &normalized, const GLsizei &stride,
                     const GLvoid *pointer);

  VertexArray &operator=(VertexArray &&other) {
    handle_ = std::move(other.handle_);
    return *this;
  }

  GLuint handle_;

private:
  KNIGHT_DISALLOW_COPY_AND_ASSIGN(VertexArray);
};

// struct UniformManager {
//   UniformManager(foundation::Allocator &allocator);
//   ~UniformManager();

//   UniformBase *Create(ShaderProgram &shader_program, GLint location, 
//                       const char *name, GLenum type);

//   template<typename T, size_t row_count, size_t col_count = 1>
//   Uniform<T, row_count, col_count> *Get(const ShaderProgram &shader_program, const char *name) const;
//   UniformBase *TryGet(const ShaderProgram &shader_program, GLint location) const;

//   void NotifyDirty(GLuint program_handle, const UniformBase *uniform);
//   void PushUniforms(const ShaderProgram &shader_program);

//   foundation::Allocator &allocator_;
//   foundation::Hash<UniformBase *> uniforms_;
//   foundation::Hash<const UniformBase *> dirty_uniforms_;

//  private:
//   KNIGHT_DISALLOW_COPY_AND_ASSIGN(UniformManager);
//   KNIGHT_DISALLOW_MOVE_AND_ASSIGN(UniformManager);
// };

} // namespace knight
