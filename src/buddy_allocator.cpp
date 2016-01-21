#include "buddy_allocator.h"

#include <logog.hpp>

using std::vector;

namespace knight {

namespace {

constexpr auto ceil_pow2(uint64_t v) {
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

constexpr auto ceil_pow2(uint32_t v) {
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v++;
  return v;
}

std::tuple<uint32_t, uint32_t> get_byte_bit_indices(uint32_t index) {
  auto byte_index = index >> 3;
  auto bit_index = index & 7;
  return std::make_tuple(byte_index, bit_index);
}

uint32_t get_allocation_index(uint32_t index) {
  return (index + 1) >> 1;
}

// Not ok to mark_allocated more than once, acts as toggle
void mark_allocated(gsl::span<uint8_t> allocation_bytes, uint32_t index) {
  uint32_t byte_index, bit_index;
  auto allocation_index = get_allocation_index(index);
  std::tie(byte_index, bit_index) = get_byte_bit_indices(allocation_index);

  DBUG("allocation: %u => %u (%u, %u)", index, allocation_index, byte_index, bit_index);
  allocation_bytes[byte_index] ^= (1u << bit_index);

  DBUG("allocation byte: %u", allocation_bytes[byte_index]);
}

// Not ok to mark_deallocated more than once, acts as toggle
bool mark_deallocated(gsl::span<uint8_t> allocation_bytes, uint32_t index) {
  mark_allocated(allocation_bytes, index);

  uint32_t byte_index, bit_index;
  std::tie(byte_index, bit_index) = get_byte_bit_indices(get_allocation_index(index));
  return (allocation_bytes[byte_index] & (1u << bit_index)) != 0;
}

void mark_split(gsl::span<uint8_t> split_bytes, uint32_t index) {
  uint32_t byte_index, bit_index;
  std::tie(byte_index, bit_index) = get_byte_bit_indices(index);

  split_bytes[byte_index] |= (1u << bit_index);
}

void mark_not_split(gsl::span<uint8_t> split_bytes, uint32_t index) {
  uint32_t byte_index, bit_index;
  std::tie(byte_index, bit_index) = get_byte_bit_indices(index);

  split_bytes[byte_index] &= ~(1u << bit_index);
}

} // namespace

BuddyAllocator::BuddyAllocator(void *buffer, uint64_t size)
  : total_size_{ceil_pow2(size)},
    height_{depth_of_block_size(kLeafSize)},
    buffer_start_{static_cast<gsl::byte *>(buffer)},
    tracking_start_{reinterpret_cast<uint8_t *>(buffer_start_ + total_size_ - size)},
    free_list_{} {
  Expects(height_ <= kMaxHeight);

  DBUG("%u %lu", height_, total_size_);

  auto buffer_end = buffer_start_ + total_size_;
  //std::fill(buffer_start_, buffer_end, gsl::byte{});

  free_list_.fill(nullptr);
  free_list_[0] = new (buffer_start_) Node{nullptr, nullptr};

  const auto tracking_count = tracking_byte_count();
  auto tracking_leaf_blocks = (tracking_count / kLeafSize) + 1;

  auto temp_tracking_location = reinterpret_cast<uint8_t *>(buffer_end - tracking_count);
  auto temp_tracking_bytes = gsl::as_span(temp_tracking_location, tracking_count);

  for (auto i = 0; i < tracking_leaf_blocks; i++) {
    allocate_at_depth(temp_tracking_bytes, height_);
  }

  std::copy(temp_tracking_bytes.begin(), temp_tracking_bytes.end(), tracking_bytes().begin());
  //std::fill(temp_tracking_span.begin(), temp_tracking_span.end(), 0u);

  // for (auto &&byte : temp_tracking_span) {
  //   DBUG("byte: %u", byte);
  // }
}

gsl::span<uint8_t> BuddyAllocator::tracking_bytes() {
  return gsl::as_span<uint8_t>(
    reinterpret_cast<uint8_t *>(tracking_start_),
    tracking_byte_count());
}

gsl::span<const uint8_t> BuddyAllocator::tracking_bytes() const {
  return const_cast<BuddyAllocator *>(this)->tracking_bytes();
}

void *BuddyAllocator::allocate(uint32_t size, uint32_t align) {
  auto block_size = ceil_pow2(size);
  auto depth = depth_of_block_size(block_size);
  DBUG("allocate()");
  return allocate_at_depth(tracking_bytes(), depth);
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

uint32_t BuddyAllocator::tracking_byte_count() const {
  return btree::nodes_at_depth(height_ + 1) >> 3;
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
  union {
    uint32_t value;
    uint8_t bytes[4];
  } tracking;

  tracking.value = 0;

  auto half_tracking_bytes = tracking_byte_count() / 2;
  auto split_tracking_bytes = tracking_bytes().subspan(0, half_tracking_bytes);

  std::copy(split_tracking_bytes.begin(), split_tracking_bytes.end(), tracking.bytes);

  auto block_index = index(ptr, depth);
  return tracking.value & (1u << block_index);
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

void BuddyAllocator::record_tracking_bits(uint32_t allocation_bits, uint32_t split_bits) {
  auto half_tracking_bytes = tracking_byte_count() >> 1;

  union {
    uint32_t value;
    uint8_t bytes[4];
  } tracking;

  tracking.value = split_bits;
  DBUG("Split bytes");
  DBUG("%u", tracking.bytes[0]);
  DBUG("%u", tracking.bytes[1]);
  DBUG("%u", tracking.bytes[2]);
  DBUG("%u", tracking.bytes[3]);

  auto tracking_byte = reinterpret_cast<uint8_t *>(buffer_start_);
  for (auto i = 0; i < half_tracking_bytes; i++) {
    tracking_byte[i] |= tracking.bytes[i];
  }

  tracking.value = allocation_bits;
  DBUG("Allocated bytes");
  DBUG("%u", tracking.bytes[0]);
  DBUG("%u", tracking.bytes[1]);
  DBUG("%u", tracking.bytes[2]);
  DBUG("%u", tracking.bytes[3]);

  tracking_byte += half_tracking_bytes;
  for (auto i = 0; i < half_tracking_bytes; i++) {
    tracking_byte[i] ^= tracking.bytes[i];
  }
}

void *BuddyAllocator::allocate_at_depth(gsl::span<uint8_t> tracking_bytes, uint32_t depth) {
  Expects(depth >= 0 && depth <= height_);
  Expects(tracking_bytes.length() > 0);

  auto half_tracking_byte_count = tracking_bytes.length() / 2;
  auto split_bytes = tracking_bytes.subspan(0, half_tracking_byte_count);
  auto allocation_bytes = tracking_bytes.subspan(half_tracking_byte_count);

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
    DBUG("Allocating index: %u depth: %u", ancestor_index, ancestor_depth);
    mark_allocated(allocation_bytes, ancestor_index);


    split_and_set_free(block, ancestor_depth);
    mark_split(split_bytes, ancestor_index);
  }

  auto block = pop_free_list(depth);
  auto block_index = index(block, depth);

  DBUG("Allocating index: %u depth: %u", block_index, depth);
  mark_allocated(allocation_bytes, block_index);


  Ensures(block != nullptr);
  return block;
}

void BuddyAllocator::deallocate_at_depth(gsl::not_null<void *> ptr, uint32_t depth) {
  Expects(depth >= 0 && depth <= height_);

  auto index = this->index(ptr, depth);

  DBUG("Deallocating %u at depth %u", index, depth);

  auto half_tracking_bytes = tracking_byte_count() / 2;
  auto allocation_bytes = tracking_bytes().subspan(half_tracking_bytes);

  if (depth < height_) {
    DBUG("Marking not split");
    auto split_bytes = tracking_bytes().subspan(0, half_tracking_bytes);
    mark_not_split(split_bytes, index);
  }

  auto buddy_is_allocated = mark_deallocated(allocation_bytes, index);
  if (buddy_is_allocated || depth == 0) {
    DBUG("Buddy is allocated or at root, adding self to free list");
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
      DBUG("Removing buddy from free list");
      buddy_node->previous->next = buddy_node->next;
    } else {
      DBUG("Removing buddy from free list, now empty");
      free_list_[depth] = nullptr;
    }

    deallocate_at_depth(ptr, depth - 1);
  }
}

} // namespace knight
