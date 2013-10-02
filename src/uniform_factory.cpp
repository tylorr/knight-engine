#include "uniform_factory.h"

#include <map>

using std::map;
using std::string;

namespace UniformFactory {

namespace {
  map<string, Uniform *> global_register_;
}

Uniform *Create(char *name, UniformType uniformType) {
  Uniform *uniform = nullptr;

  if (global_register_.find(name) != global_register_.end())

  switch(uniformType) {
    case UniformType::FLOAT_: {
      uniform = new UniformFloat(name);
      break;
    }
    case UniformType::FLOAT_VEC2: {
      uniform = new UniformVec2(name);
      break;
    }
    case UniformType::FLOAT_VEC3: {
      uniform = new UniformVec3(name);
      break;
    }
    case UniformType::FLOAT_VEC4: {
      uniform = new UniformVec4(name);
      break;
    }
    case UniformType::INT_: {
      uniform = new UniformInt(name);
      break;
    }
  }

  return uniform;
}

void RegisterGlobal(const std::string &name) {
  if (global_register_.find(name) == global_register_.end()) {
    // global_register_[name] = ;
  }
}

}; // namespace UniformFactory

