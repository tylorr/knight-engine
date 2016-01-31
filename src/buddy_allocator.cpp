#include "buddy_allocator.h"

#include <logog.hpp>

namespace knight {

using namespace bit_span::detail;

namespace {

constexpr auto pow2_ceil(uint64_t v) {
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v |= v >> 32;
  v++;
  return v;
}

constexpr auto pow2_ceil(uint32_t v) {
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v++;
  return v;
}

uint32_t get_allocation_index(uint32_t index) {
  return (index + 1) >> 1;
}

// Not ok to mark_allocated more than once, acts as toggle
void mark_allocated(AllocationBits allocation_bits, uint32_t index) {
  auto allocation_index = get_allocation_index(index);
  allocation_bits.flip(allocation_index);
}

// Not ok to mark_deallocated more than once, acts as toggle
bool mark_deallocated(AllocationBits allocation_bits, uint32_t index) {
  mark_allocated(allocation_bits, index);
  return allocation_bits.test(get_allocation_index(index));
}

SplitBits split_bits(TrackingBits tracking_bits) {
  return tracking_bits.subspan(0, tracking_bits.size() / 2);
}

AllocationBits allocation_bits(TrackingBits tracking_bits) {
  auto half_tracking_bits = tracking_bits.size() / 2;
  auto half_tracking_bytes = half_tracking_bits / 8;
  return tracking_bits.subspan(half_tracking_bytes, half_tracking_bits);
}

} // namespace

BuddyAllocator::BuddyAllocator(void *buffer, uint64_t size)
  : total_size_{pow2_ceil(size)},
    height_{depth_of_block_size(kLeafSize)},
    buffer_start_{static_cast<gsl::byte *>(buffer)},
    tracking_start_{reinterpret_cast<uint8_t *>(buffer_start_ + total_size_ - size)},
    free_list_{} {
  Expects(height_ <= kMaxHeight);


  auto buffer_end = buffer_start_ + total_size_;
  //std::fill(buffer_start_, buffer_end, gsl::byte{});

  free_list_.fill(nullptr);
  free_list_[0] = new (buffer_start_) Node{nullptr, nullptr};

  const auto tracking_bit_count = this->tracking_bit_count();
  const auto tracking_byte_count = tracking_bit_count / 8;
  auto tracking_leaf_blocks = (tracking_byte_count / kLeafSize) + 1;

  auto temp_tracking_location = reinterpret_cast<uint8_t *>(buffer_end - tracking_byte_count);
  auto temp_tracking_bits = TrackingBits{temp_tracking_location, tracking_bit_count};

  for (auto i = 0; i < tracking_leaf_blocks; i++) {
    allocate_at_depth(temp_tracking_bits, height_);
  }

  std::copy(temp_tracking_bits.block_begin(), temp_tracking_bits.block_end(), tracking_bits().block_begin());
  temp_tracking_bits.reset();

  // for (auto &&byte : temp_tracking_span) {
  // }
}

TrackingBits BuddyAllocator::tracking_bits() {
  return const_cast<const BuddyAllocator *>(this)->tracking_bits();
}

const TrackingBits BuddyAllocator::tracking_bits() const {
  return {tracking_start_, tracking_bit_count()};
}

void *BuddyAllocator::allocate(uint32_t size, uint32_t align) {
  auto block_size = pow2_ceil(size);
  auto depth = depth_of_block_size(block_size);
  return allocate_at_depth(tracking_bits(), depth);
}

void BuddyAllocator::deallocate(void *ptr) {
  if (ptr == nullptr) return;
  deallocate_at_depth(ptr, get_depth(ptr));
}

void BuddyAllocator::deallocate(void *ptr, uint32_t size) {
  if (ptr == nullptr) return;
  deallocate_at_depth(ptr, depth_of_block_size(size));
}

uint32_t BuddyAllocator::allocated_size(void *p) {
  return 0;
}

uint32_t BuddyAllocator::total_allocated() { return 0; }

void *BuddyAllocator::allocation_base(void *p) { return nullptr; }

uint32_t BuddyAllocator::tracking_bit_count() const {
  return btree::nodes_at_depth(height_ + 1);
}

uint32_t BuddyAllocator::depth_of_block_size(uint32_t size) const {
  return std::min(gsl::narrow_cast<uint32_t>(std::log2(total_size_ / size)), height_);
}

uint64_t BuddyAllocator::block_size_at_depth(uint32_t depth) const {
  Expects(depth >= 0 && depth <= height_);
  return total_size_ / btree::nodes_at_depth(depth);
}

uint32_t BuddyAllocator::index_at_depth(gsl::not_null<void *> ptr, uint32_t depth) const {
  Expects(depth >= 0 && depth <= height_);
  return (static_cast<gsl::byte *>(ptr.get()) - buffer_start_) / block_size_at_depth(depth);
}

void *BuddyAllocator::block_from_index_at_depth(uint32_t index, uint32_t depth) const {
  return index * block_size_at_depth(depth) + buffer_start_;
}

uint32_t BuddyAllocator::index(gsl::not_null<void *> ptr, uint32_t depth) const {
  Expects(depth >= 0 && depth <= height_);
  return btree::nodes_at_depth(depth) + index_at_depth(ptr, depth) - 1;
}

bool BuddyAllocator::check_split_block(gsl::not_null<void *> ptr, uint32_t depth) const {
  Expects(depth < height_);
  auto split_tracking_bits = split_bits(tracking_bits());
  auto block_index = index(ptr, depth);
  return split_tracking_bits.test(block_index);
}

uint32_t BuddyAllocator::get_depth(gsl::not_null<void *> ptr) const {
  auto depth = height_;
  while (depth > 0u) {
    if (check_split_block(ptr, depth - 1u)) {
      return depth;
    }
    depth = depth - 1u;
  }
  return 0u;
}

void *BuddyAllocator::pop_free_list(uint32_t depth) {
  Expects(free_list_[depth] != nullptr);

  auto block = free_list_[depth];

  auto node = static_cast<Node *>(free_list_[depth]);
  free_list_[depth] = node->next;

  if (node->next != nullptr) {
    node->next->previous = nullptr;
  }

  return block;
}

void BuddyAllocator::split_and_set_free(gsl::not_null<void *> ptr, uint32_t parent_depth) {
  Expects(parent_depth >= 0 && parent_depth < height_);

  auto parent_block = static_cast<gsl::byte *>(ptr.get());

  auto depth = parent_depth + 1;

  auto block_size = block_size_at_depth(depth);
  auto first_block = parent_block;
  auto second_block = parent_block + block_size;

  auto first_node = new (first_block) Node{nullptr, nullptr};
  auto second_node = new (second_block) Node{first_node, nullptr};
  first_node->next = second_node;

  free_list_[depth] = first_node;
}

void *BuddyAllocator::allocate_at_depth(TrackingBits tracking_bits, uint32_t depth) {
  Expects(depth >= 0 && depth <= height_);
  Expects(tracking_bits.size() > 0);

  auto split_tracking_bits = split_bits(tracking_bits);
  auto allocation_tracking_bits = allocation_bits(tracking_bits);

  int32_t first_free_depth = depth;
  while (first_free_depth >= 0 && free_list_[first_free_depth] == nullptr) {
    first_free_depth--;
  }

  if (first_free_depth < 0) {
    WARN("Buddy allocator cannot allocate at depth %u", depth);
    return nullptr;
  }

  for (auto ancestor_depth = first_free_depth; ancestor_depth < depth; ancestor_depth++) {
    auto block = pop_free_list(ancestor_depth);

    auto ancestor_index = index(block, ancestor_depth);
    mark_allocated(allocation_tracking_bits, ancestor_index);

    split_and_set_free(block, ancestor_depth);
    split_tracking_bits.set(ancestor_index);
  }

  auto block = pop_free_list(depth);
  auto block_index = index(block, depth);

  mark_allocated(allocation_tracking_bits, block_index);

  Ensures(block != nullptr);
  return block;
}

void BuddyAllocator::deallocate_at_depth(gsl::not_null<void *> ptr, uint32_t depth) {
  Expects(depth >= 0 && depth <= height_);

  auto index = this->index(ptr, depth);

  if (depth < height_) {
    split_bits(tracking_bits()).reset(index);
  }

  auto buddy_is_allocated = mark_deallocated(allocation_bits(tracking_bits()), index);

  if (buddy_is_allocated || depth == 0) {
    Node *original_front = nullptr;
    if (free_list_[depth] != nullptr) {
      original_front = new (free_list_[depth]) Node{};
    }
    free_list_[depth] = new (ptr.get()) Node {nullptr, original_front};
  } else {
    auto buddy_index = (index & 1) == 0 ? index - 1 : index + 1;
    auto buddy_ptr = block_from_index_at_depth(buddy_index, depth);
    auto buddy_node = new (buddy_ptr) Node{};

    if (buddy_node->previous != nullptr) {
      buddy_node->previous->next = buddy_node->next;
    } else {
      free_list_[depth] = nullptr;
    }

    deallocate_at_depth(ptr, depth - 1);
  }
}

} // namespace knight
