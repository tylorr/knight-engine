#ifndef KNIGHT_UNIFORM_FACTORY_H_
#define KNIGHT_UNIFORM_FACTORY_H_

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
  typedef std::function<UniformBase *(const std::string &)> UniformFactoryFunc;

  std::map<UniformNameType, UniformBase *> uniforms_;
  std::map<GLenum, UniformFactoryFunc> uniform_factories_;

  template<typename T, size_t row_count, size_t col_count = 1>
  void Register(const GLenum &type);

  KNIGHT_DISALLOW_COPY_AND_ASSIGN(UniformFactory);
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

template<typename T, size_t row_count, size_t col_count>
void UniformFactory::Register(const GLenum &type) {
  uniform_factories_[type] = [] (const std::string &name) { 
    return new Uniform<T, row_count, col_count>(name);
  };
}

} // namespace knight

#endif // KNIGHT_UNIFORM_FACTORY_H_
