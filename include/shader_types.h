#pragma once

#include "common.h"

#include <collection_types.h>
#include <memory_types.h>

#include <GL/glew.h>

namespace knight {

class ShaderProgram;
class UniformBase;

template<typename T, size_t row_count, size_t col_count>
class Uniform;

class UniformFactory {
 public:
  UniformFactory(foundation::Allocator &allocator);
  ~UniformFactory();

  UniformBase *Create(ShaderProgram &shader_program, const GLint &location, 
                      const char *name, const GLenum &type);

  template<typename T, size_t row_count, size_t col_count = 1>
  Uniform<T, row_count, col_count> *Get(const char *name) const;

  UniformBase *TryGet(const char *name, GLenum type) const;

  template<typename T, size_t row_count, size_t col_count = 1>
  struct GetType {
    static const GLenum value;
    static const char *name;
  };

  foundation::Hash<UniformBase *> uniforms_;
  foundation::Allocator &allocator_;

  KNIGHT_DISALLOW_COPY_AND_ASSIGN(UniformFactory);
  KNIGHT_DISALLOW_MOVE_AND_ASSIGN(UniformFactory);
};

} // namespace knight
