#pragma once

#include "pointers.h"
#include "game_memory.h"

#include <memory.h>

#include <cstring>

namespace knight {

namespace memory_block {

  template<typename ...Ts>
  pointer<char[]> grow_contiguous(foundation::Allocator &allocator, uint32_t original_capacity, uint32_t capacity, Ts&&... args);

  // TODO: Find a good home for this
  inline auto align_forward(uintptr_t offset, size_t align) {
    return uintptr_t{((offset + align - 1) / align) * align};
  }

  namespace detail {
    auto grow_contiguous_helper(
        foundation::Allocator &allocator,
        uint32_t original_capacity, 
        uint32_t capacity,
        uint32_t total_bytes) {
      return allocate_unique<char[]>(allocator, total_bytes);
    }

    template<typename T, typename ...Ts>
    pointer<char[]> 
      grow_contiguous_helper(
        foundation::Allocator &allocator,
        uint32_t original_capacity, 
        uint32_t capacity,
        uint32_t offset,
        T *&ptr,
        Ts&&... args) {
      auto align = alignof(T);
      auto aligned_offset = align_forward(offset, align);
      auto end_offset = aligned_offset + capacity * sizeof(T);

      auto memory_block = 
        grow_contiguous_helper(
          allocator,
          original_capacity,
          capacity,
          end_offset,
          args...);

      auto original_ptr = ptr;
      ptr = reinterpret_cast<T *>(memory_block.get() + aligned_offset);
      memcpy(ptr, original_ptr, original_capacity * sizeof(T));

      return std::move(memory_block);
    }
  } // namespace detail

  template<typename ...Ts>
  pointer<char[]> 
    grow_contiguous(
      foundation::Allocator &allocator,
      uint32_t original_capacity, 
      uint32_t capacity, 
      Ts&&... args) {
    XASSERT(capacity >= original_capacity, "Cannot shrink contiguous memory");
    return detail::grow_contiguous_helper(allocator, original_capacity, capacity, 0u, args...);
  }

} // namespace memory_block

} // namespace knight
