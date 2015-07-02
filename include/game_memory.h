#pragma once

#include "common.h"
#include "game_memory_types.h"

#include <memory.h>

namespace knight {

class GameAllocatorState {
 public:
  foundation::HeapAllocator *default_allocator;
  foundation::ScratchAllocator *scratch_allocator;

  virtual ~GameAllocatorState() { }
};

namespace game_memory_internal {

  void Initialize(GameAllocatorState *allocator_state);

}

namespace game_memory {

  template<typename T>
  bool Initialize(GameMemory *game_memory, T **allocator_state_ptr) {
    using namespace foundation;

    XASSERT(sizeof(T) <= game_memory->memory_size - game_memory->temporary_memory_size, 
            "GameAllocatorState is too large for allocated game memory");

    *allocator_state_ptr = static_cast<T *>(game_memory->memory);
    auto allocator_state = *allocator_state_ptr;

    if (!game_memory->is_initialized) {
      auto buffer = static_cast<char *>(game_memory->memory) + sizeof(T);
      
      auto default_allocator = new (buffer) HeapAllocator(
          buffer + sizeof(HeapAllocator), 
          game_memory->memory_size - sizeof(HeapAllocator) - sizeof(T));

      allocator_state->default_allocator = default_allocator;

      auto scratch_allocator = default_allocator->make_new<ScratchAllocator>(
          *allocator_state->default_allocator, 
          game_memory->temporary_memory_size);

      allocator_state->scratch_allocator = scratch_allocator;

      game_memory->is_initialized = true;
    }

    game_memory_internal::Initialize(*allocator_state_ptr);

    return game_memory->is_initialized;
  }

  /// Returns a default memory allocator that can be used for most allocations.
  ///
  /// You need to call Initialize() for this allocator to be available.
  foundation::Allocator &default_allocator();

  /// Returns a "scratch" allocator that can be used for temporary short-lived memory
  /// allocations. The scratch allocator uses a ring buffer of size scratch_buffer_size
  /// to service the allocations.
  ///
  /// If there is not enough memory in the buffer to match requests for scratch
  /// memory, memory from the default_allocator will be returned instead.
  foundation::Allocator &default_scratch_allocator();

  /// Shuts down the global memory allocators created by Initialize().
  void Shutdown();

} // namespace game_memory

} // namespace knight
