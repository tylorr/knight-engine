#ifndef KNIGHT_UNIFORM_FACTORY_H_
#define KNIGHT_UNIFORM_FACTORY_H_

#include "uniform.h"

#include <GL/glew.h>

#include <string>
#include <utility>
#include <map>
#include <exception>

namespace knight {

class ShaderProgram;

class UniformFactory {
 public:
  UniformBase *Create(ShaderProgram *shader_program, const GLint &location, 
                      const char *name, const GLenum &type);

  template<typename T, size_t row_count, size_t col_count = 1>
  Uniform<T, row_count, col_count> *Get(const std::string &name);

 private:
  template<typename T, size_t row_count, size_t col_count = 1>
  struct GetType {
    static const GLenum value;
    static const char *string;
  };

  typedef std::pair<std::string, GLenum> UniformNameType;

  std::map<UniformNameType, UniformBase *> uniforms_;
};

template<typename T, size_t row_count, size_t col_count>
Uniform<T, row_count, col_count> *UniformFactory::Get(const std::string &name) {
  GLenum type = GetType<T, row_count, col_count>::value;
  auto it = uniforms_.find(std::make_pair(name, type));

  if (it != uniforms_.end()) {
    UniformBase *uniform_base = (*it).second;
    return static_cast<Uniform<T, row_count, col_count> *>(uniform_base);
  } else {
    ERR("No active uniform of type '%s' with name '%s'", 
        GetType<T, row_count, col_count>::string, name.c_str());

    throw std::invalid_argument(name);
  }
}

} // namespace knight

#endif // KNIGHT_UNIFORM_FACTORY_H_
