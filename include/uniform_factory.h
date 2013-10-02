#ifndef UNIFORM_FACTORY_H_
#define UNIFORM_FACTORY_H_

#include "uniform.h"

#include <string>

namespace UniformFactory {

Uniform *Create(char *, UniformType);

void RegisterGlobal(const std::string &);

};

#endif // UNIFORM_FACTORY_H_
