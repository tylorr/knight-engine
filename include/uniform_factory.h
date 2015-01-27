#pragma once

#include "shader_types.h"

namespace knight {

template<typename T, size_t row_count, size_t col_count>
Uniform<T, row_count, col_count> *UniformFactory::Get(const char *name) const {
  auto type = GetType<T, row_count, col_count>::value;
  auto uniform_base = TryGet(name, type);

  XASSERT(uniform_base != nullptr, 
    "No active uniform of type '%s' with name '%s'", 
    GetType<T, row_count, col_count>::name, name);

  return static_cast<Uniform<T, row_count, col_count> *>(uniform_base);
}

} // namespace knight
