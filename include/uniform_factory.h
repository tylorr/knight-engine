#pragma once

#include "uniform.h"

#include <GL/glew.h>

#include <string>
#include <utility>
#include <map>
#include <exception>
#include <functional>

namespace knight {

class ShaderProgram;

class UniformFactory {
 public:
  UniformFactory();
  ~UniformFactory();

  UniformBase *Create(ShaderProgram &shader_program, const GLint &location, 
                      const char *name, const GLenum &type);

  template<typename T, size_t row_count, size_t col_count = 1>
  Uniform<T, row_count, col_count> *Get(const std::string &name);

 private:
  template<typename T, size_t row_count, size_t col_count = 1>
  struct GetType {
    static const GLenum value;
    static const char *string;
  };

  using UniformNameType = std::pair<std::string, GLenum>;
  using UniformFactoryFunc = std::function<UniformBase *(const std::string &)>;

  std::map<UniformNameType, UniformBase *> uniforms_;
  std::map<GLenum, UniformFactoryFunc> uniform_factories_;

  template<typename T, size_t row_count, size_t col_count = 1>
  void RegisterUniformType(const GLenum &type);

  KNIGHT_DISALLOW_COPY_AND_ASSIGN(UniformFactory);
  KNIGHT_DISALLOW_MOVE_AND_ASSIGN(UniformFactory);
};

template<typename T, size_t row_count, size_t col_count>
Uniform<T, row_count, col_count> *UniformFactory::Get(const std::string &name) {
  auto type = GetType<T, row_count, col_count>::value;
  auto it = uniforms_.find(std::make_pair(name, type));

  XASSERT(it != uniforms_.end(), 
    "No active uniform of type '%s' with name '%s'", 
    GetType<T, row_count, col_count>::string, name.c_str());

  auto uniform_base = (*it).second;
  return static_cast<Uniform<T, row_count, col_count> *>(uniform_base);
}

template<typename T, size_t row_count, size_t col_count>
void UniformFactory::RegisterUniformType(const GLenum &type) {
  uniform_factories_[type] = [] (const std::string &name) { 
    return new Uniform<T, row_count, col_count>(name);
  };
}

} // namespace knight
