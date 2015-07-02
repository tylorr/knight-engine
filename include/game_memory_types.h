#pragma once

#include <memory_types.h>

#include <cstdlib>

namespace knight {

struct GameMemory {
  bool is_initialized;

  size_t memory_size;
  size_t temporary_memory_size;
  void *memory;
};

class GameAllocatorState;

} // namespace knight
