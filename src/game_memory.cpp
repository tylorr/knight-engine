#include "game_memory.h"

using namespace foundation;

namespace knight {

namespace {
  GameAllocatorState *game_allocator_state_;
}

namespace game_memory_internal {

  void Initialize(GameAllocatorState *allocator_state) {
    game_allocator_state_ = allocator_state;
  }

}

namespace game_memory {
  
  foundation::Allocator &default_allocator() {
    return *game_allocator_state_->default_allocator;
  }

  foundation::Allocator &default_scratch_allocator() {
    return *game_allocator_state_->scratch_allocator;
  }

  void Shutdown() {
    if (game_allocator_state_ != nullptr) {
      game_allocator_state_->default_allocator->make_delete(game_allocator_state_->scratch_allocator);
      
      // TODO TR: Find out why I can't call destructor
      //game_allocator_state_->default_allocator->~HeapAllocator();
      
      game_allocator_state_ = nullptr;
    }
  }

} // namespace game_memory

} // namespace knight
