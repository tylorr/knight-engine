#pragma once

#include "common.h"
#include "bit_span.h"

#include <gsl.h>
#include <memory.h>

namespace knight {

namespace btree {

constexpr auto nodes_at_depth(uint32_t depth) {
  return 1ull << depth;
}

constexpr auto node_count(uint32_t height) {
  return (1ull << (height + 1)) - 1;
}

} // namespace btree

namespace bit_span {
namespace detail {
  struct TrackingBits : public BitSpan<uint8_t> { using BitSpan<uint8_t>::BitSpan; };
  struct AllocationBits : public BitSpan<uint8_t> { using BitSpan<uint8_t>::BitSpan; };
  struct SplitBits : public BitSpan<uint8_t> { using BitSpan<uint8_t>::BitSpan; };
}
}

class BuddyAllocator : public foundation::Allocator {
 public:
  BuddyAllocator(void *buffer, uint64_t size);

  virtual void *allocate(uint32_t size, uint32_t align = DEFAULT_ALIGN) override;
  virtual void deallocate(void *p) override;
  void deallocate(void *p, uint32_t size);

  virtual uint32_t allocated_size(void *p) override;
  virtual uint32_t total_allocated() override;
  virtual void *allocation_base(void *p) override;

 private:
  struct Node {
    Node *previous;
    Node *next;
  };

  using TrackingBits = bit_span::detail::TrackingBits;

  static constexpr uint32_t kMaxHeight = (1 << 5) - 1;
  static constexpr uint32_t kLeafSize = 1 << 7;
  static_assert(kLeafSize >= sizeof(Node), "Leaf size too small for linked list");

  const uint64_t total_size_;
  const uint32_t height_;
  gsl::byte * const buffer_start_;
  uint8_t * const tracking_start_;
  std::array<void *, kMaxHeight + 1> free_list_;

  uint32_t tracking_bit_count() const;
  TrackingBits tracking_bits();
  const TrackingBits tracking_bits() const;

  uint32_t depth_of_block_size(uint32_t size) const;
  uint64_t block_size_at_depth(uint32_t depth) const;
  uint32_t index_at_depth(gsl::not_null<void *> ptr, uint32_t depth) const;
  uint32_t index(gsl::not_null<void *> ptr, uint32_t depth) const;
  void *block_from_index_at_depth(uint32_t index, uint32_t depth) const;

  bool check_split_block(gsl::not_null<void *> ptr, uint32_t depth) const;
  uint32_t get_depth(gsl::not_null<void *> ptr) const;

  void *pop_free_list(uint32_t depth);
  void split_and_set_free(gsl::not_null<void *> ptr, uint32_t parent_depth);

  void *allocate_at_depth(TrackingBits tracking_bits, uint32_t depth);
  void deallocate_at_depth(gsl::not_null<void *> ptr, uint32_t depth);
};

} // namespace knight
