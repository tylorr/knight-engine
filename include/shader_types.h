#pragma once

#include <cstdlib>

namespace knight {

class UniformBase;
template<typename T, size_t row_count, size_t col_count>
class Uniform;

class MaterialManager;
class Material;
class BufferObject;
class ArrayObject;


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
