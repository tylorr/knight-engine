#pragma once

#include <gsl.h>

#include <limits>
#include <algorithm>

namespace knight {

template<typename BlockType = uint8_t>
class BitSpan {
 public:
  using block_type = BlockType;
  using size_type = std::size_t;
  using block_width_type = int32_t;

  static constexpr block_width_type kBitsPerBlock = std::numeric_limits<BlockType>::digits;

  BitSpan(BlockType *ptr, size_type size);

  constexpr BitSpan(const BitSpan &) = default;
  constexpr BitSpan(BitSpan &&) = default;

  constexpr BitSpan &operator=(const BitSpan &) = default;
  constexpr BitSpan &operator=(BitSpan &&) = default;

  size_type size() const;
  bool empty() const;

  BitSpan &set();
  BitSpan &set(size_type position, bool value = true);

  BitSpan &reset();
  BitSpan &reset(size_type position);

  BitSpan &flip();
  BitSpan &flip(size_type position);

  bool test(size_type position) const;
  bool all() const;
  bool any() const;
  bool none() const;

  BitSpan subspan(std::ptrdiff_t block_offset, std::ptrdiff_t count = gsl::dynamic_range) const;

 private:
  using block_span_type = gsl::span<BlockType>;

  block_span_type block_span_;
  size_type bit_count_;

  static constexpr size_type block_index(size_type position) {
    return position / kBitsPerBlock;
  }

  static constexpr block_width_type bit_index(size_type position) {
    return static_cast<block_width_type>(position % kBitsPerBlock);
  }

  static constexpr BlockType bit_mask(size_type position) {
    return BlockType{1} << bit_index(position);
  }

  static constexpr typename block_span_type::size_type
  calculate_block_count(size_type bit_count) {
    auto block_count = bit_count / kBitsPerBlock;

    if (bit_count % kBitsPerBlock != 0) {
      block_count++;
    }

    return block_count;
  }

  static constexpr BlockType all_ones() {
    return static_cast<BlockType>(~0);
  }

  static constexpr BlockType extra_bits_mask(size_type extra_bits) {
    return ~(all_ones() << extra_bits);
  }

  block_width_type count_extra_bits() const {
    return bit_index(size());
  }

  void zero_unused_bits();
};

template<typename BlockType>
BitSpan<BlockType>::BitSpan(BlockType *ptr, size_type bit_count)
  : block_span_{ptr, calculate_block_count(bit_count)},
    bit_count_{bit_count} { }

template<typename BlockType>
inline typename BitSpan<BlockType>::size_type
BitSpan<BlockType>::size() const {
  return bit_count_;
}

template<typename BlockType>
inline bool BitSpan<BlockType>::empty() const {
  return size() == 0;
}

template<typename BlockType>
BitSpan<BlockType> &BitSpan<BlockType>::set() {
  std::fill(block_span_.begin(), block_span_.end(), ~BlockType{0});
  zero_unused_bits();
  return *this;
}

template<typename BlockType>
BitSpan<BlockType> &BitSpan<BlockType>::set(size_type position, bool value) {
  Expects(position < bit_count_);
  if (value) {
    block_span_[block_index(position)] |= bit_mask(position);
  } else {
    reset(position);
  }

  return *this;
}

template<typename BlockType>
BitSpan<BlockType> &BitSpan<BlockType>::reset() {
  std::fill(block_span_.begin(), block_span_.end(), BlockType{0});
  return *this;
}

template<typename BlockType>
BitSpan<BlockType> &BitSpan<BlockType>::reset(size_type position) {
  Expects(position < bit_count_);
  block_span_[block_index(position)] &= ~bit_mask(position);

  return *this;
}

template<typename BlockType>
BitSpan<BlockType> &BitSpan<BlockType>::flip() {
  for (auto &&block : block_span_) {
    block = ~block;
  }
  zero_unused_bits();
  return *this;
}

template<typename BlockType>
BitSpan<BlockType> &BitSpan<BlockType>::flip(size_type position) {
  Expects(position < bit_count_);
  block_span_[block_index(position)] ^= bit_mask(position);

  return *this;
}

template<typename BlockType>
bool BitSpan<BlockType>::test(size_type position) const {
  Expects(position < bit_count_);
  return (block_span_[block_index(position)] & bit_mask(position)) != 0;
}

template<typename BlockType>
bool BitSpan<BlockType>::all() const {
  if (empty()) {
    return true;
  }

  const auto extra_bits = count_extra_bits();
  auto all_ones = this->all_ones();

  if (extra_bits == 0) {
    for (auto const &block : block_span_) {
      if (block != all_ones) {
        return false;
      }
    }
  } else {
    auto block_count = block_span_.size();
    for (auto i = 0; i < block_count - 1; i++) {
      if (block_span_[i] != all_ones) {
        return false;
      }
    }

    const auto last_block = block_span_[block_span_.size() - 1];
    if (last_block != extra_bits_mask(extra_bits)) {
      return false;
    }
  }

  return true;
}

template<typename BlockType>
bool BitSpan<BlockType>::any() const {
  for (auto const &block : block_span_) {
    if (block > 0) {
      return true;
    }
  }
  return false;
}

template<typename BlockType>
bool BitSpan<BlockType>::none() const {
  return !any();
}

template<typename BlockType>
void BitSpan<BlockType>::zero_unused_bits() {
  const auto extra_bits = count_extra_bits();
  if (extra_bits != 0) {
    block_span_[block_span_.size() - 1] &= extra_bits_mask(extra_bits);
  }
}

template<typename BlockType>
BitSpan<BlockType> BitSpan<BlockType>::subspan(std::ptrdiff_t block_offset, std::ptrdiff_t count) const {
  Expects((block_offset >= 0 && block_offset <= this->size()) && (count <= this->size() - block_offset));
  return {block_span_.data() + block_offset, count == gsl::dynamic_range ? this->size() - block_offset : count};
}

} // namespace knight
