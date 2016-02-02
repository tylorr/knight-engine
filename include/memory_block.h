#pragma once

#include "pointers.h"
#include "template_util.h"

#include <memory.h>

#include <cstring>

namespace knight {

namespace memory_block {

template<typename ...Ts>
Pointer<char[]> grow_contiguous(foundation::Allocator &allocator, uint32_t original_capacity, uint32_t capacity, Ts&&... args);

template<std::size_t N, typename ...Args>
void pack_data(char (&ptr)[N], Args&&... args);

template<typename T, typename ...Args>
void unpack_data(const void *ptr, T &value, Args&&... args);

// TODO: Find a good home for this
inline auto align_forward(uintptr_t offset, size_t align) {
  return uintptr_t{((offset + align - 1) / align) * align};
}

namespace detail {
  inline auto grow_contiguous_helper(
      foundation::Allocator &allocator,
      uint32_t original_capacity,
      uint32_t capacity,
      uint32_t total_bytes) {
    return allocate_unique<char[]>(allocator, total_bytes);
  }

  template<typename T, typename ...Ts>
  Pointer<char[]>
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

  inline void pack_data_helper(char *ptr) { }

  template<typename T, typename ...Args>
  void pack_data_helper(char *ptr, T &&value, Args&&... args) {
    memcpy(ptr, &value, sizeof(T));
    pack_data_helper(ptr + sizeof(T), std::forward<Args>(args)...);
  }

} // namespace detail

template<typename ...Ts>
Pointer<char[]>
  grow_contiguous(
    foundation::Allocator &allocator,
    uint32_t original_capacity,
    uint32_t capacity,
    Ts&&... args) {
  XASSERT(capacity >= original_capacity, "Cannot shrink contiguous memory");
  return detail::grow_contiguous_helper(allocator, original_capacity, capacity, 0u, args...);
}

template<std::size_t N, typename ...Args>
void pack_data(char (&ptr)[N], Args&&... args) {
  XASSERT(sizeof_sum(args...) <= N, "Data cannot fit into buffer with size %zu", N);
  detail::pack_data_helper(ptr, std::forward<Args>(args)...);
}

inline void unpack_data(const void *ptr) { }

template<typename T, typename ...Args>
void unpack_data(const void *ptr, T &value, Args&&... args) {
  value = *reinterpret_cast<T *>(const_cast<void *>(ptr));
  auto char_ptr = (const char *)ptr;
  unpack_data(char_ptr + sizeof(T), std::forward<Args>(args)...);
}

} // namespace memory_block

} // namespace knight
