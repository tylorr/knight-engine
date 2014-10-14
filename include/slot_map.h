#pragma once

#include "common.h"

#include <memory.h>
#include <array.h>

#include <vector>
#include <cstddef>

namespace knight {

template<typename T, typename ID>
class SlotMap {
 public:
  const size_t kChunkSize = 256;

  SlotMap(foundation::Allocator &allocator)
    : slot_table_(allocator),
      free_list_(allocator) { }

  SlotMap(SlotMap &&other) 
    : slot_table_(std::move(other.slot_table_)),
      free_list_(std::move(other.free_list_)) { }

  ID Create();
  T *Get(const ID &id) const;
  void Destroy(ID id);

  SlotMap &operator=(SlotMap &&other) {
    slot_table_ = std::move(other.slot_table_);
    free_list_ = std::move(other.free_list_);
    return *this;
  }

 private:
  typedef std::unique_ptr<T[]> Chunk;

  foundation::Array<Chunk> slot_table_;
  foundation::Array<typename ID::type> free_list_;

  KNIGHT_DISALLOW_COPY_AND_ASSIGN(SlotMap);
};

template<typename T, typename ID>
ID SlotMap<T, ID>::Create() {
  // Are there no spare entities?
  if (foundation::array::empty(free_list_)) {
    auto slot_table_size = foundation::array::size(slot_table_);
    foundation::array::reserve(free_list_, kChunkSize * (slot_table_size + 1));

    // Mark entire chunk as free
    typename ID::type last_index = slot_table_size * kChunkSize;
    for (int i = kChunkSize - 1; i >= 0; --i) {
      foundation::array::push_back(free_list_, last_index + i);
    }

    // Add new chunk to table
    foundation::array::emplace_back(slot_table_, new T[kChunkSize]);
  }

  // get first free index
  typename ID::type free_index = foundation::array::back(free_list_);
  foundation::array::pop_back(free_list_);

  // get object at index and update it's index
  T *object = &slot_table_[free_index / kChunkSize][free_index % kChunkSize];
  object->id_.index = free_index;

  return object->id_;
}

template<typename T, typename ID>
T *SlotMap<T, ID>::Get(const ID &id) const {
  typename ID::type chunkIndex = id.index / kChunkSize;

  // Does the chunk exist?
  if (chunkIndex < foundation::array::size(slot_table_)) {
    T *object = &slot_table_[chunkIndex][id.index % kChunkSize];

    // If the ids (specifically the versions) match return object else NULL
    return object->id_ != id ? nullptr : object;
  } else {

    // Chunk doesn't exist return NULL
    return nullptr;
  }
}

template<typename T, typename ID>
void SlotMap<T, ID>::Destroy(ID id) {
  T *object = Get(id);

  XASSERT(object != nullptr, "Trying to delete non-existent object: %lu", id.id);

  // Increment version to generate unique id and invalidate old id
  object->id_.version++;

  foundation::array::push_back(free_list_, object->id_.index);
}

} // namespace knight
