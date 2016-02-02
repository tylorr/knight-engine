#pragma once

#include "common.h"
#include "vector.h"

#include <memory.h>

#include <cstddef>

namespace knight {

template<typename T, typename ID>
class SlotMap {
 public:
  const size_t kChunkSize = 256;

  SlotMap(foundation::Allocator &allocator)
    : slot_table_{allocator},
      free_list_{allocator} { }

  SlotMap(const SlotMap &) = delete;
  SlotMap(SlotMap &&) = default;

  SlotMap &operator=(const SlotMap &) = delete;
  SlotMap &operator=(SlotMap &&) = default;

  ID create();
  T *get(const ID &id) const;
  void destroy(ID id);

 private:
  typedef std::unique_ptr<T[]> Chunk;

  Vector<Chunk> slot_table_;
  Vector<typename ID::type> free_list_;
};

template<typename T, typename ID>
ID SlotMap<T, ID>::create() {
  // Are there no spare entities?
  if (free_list_.empty()) {
    auto slot_table_size = slot_table_.size();
    free_list_.reserve(kChunkSize * (slot_table_size + 1));

    // Mark entire chunk as free
    typename ID::type last_index = slot_table_size * kChunkSize;
    for (int i = kChunkSize - 1; i >= 0; --i) {
      free_list_.push_back(last_index + i);
    }

    // Add new chunk to table
    slot_table_.emplace_back(new T[kChunkSize]);

    // Reserve ID 0.0 (index.version) as 'null' ID
    if (slot_table_.size() == 1) {
      T &first_object = slot_table_[0][0];
      first_object.id.version = 1;
    }
  }

  // get first free index
  typename ID::type free_index = free_list_.back();
  free_list_.pop_back();

  // get object at index and update it's index
  T *object = &slot_table_[free_index / kChunkSize][free_index % kChunkSize];
  object->id.index = free_index;

  return object->id;
}

template<typename T, typename ID>
T *SlotMap<T, ID>::get(const ID &id) const {
  typename ID::type chunkIndex = id.index / kChunkSize;

  // Does the chunk exist?
  if (chunkIndex < slot_table_.size()) {
    T *object = &slot_table_[chunkIndex][id.index % kChunkSize];

    // If the ids (specifically the versions) match return object else NULL
    return object->id == id ? object : nullptr;
  } else {

    // Chunk doesn't exist return NULL
    return nullptr;
  }
}

template<typename T, typename ID>
void SlotMap<T, ID>::destroy(ID id) {
  T *object = get(id);

  XASSERT(object != nullptr, "Trying to delete non-existent object: %lu", id.id);

  // Increment version to generate unique id and invalidate old id
  object->id.version++;

  free_list_.push_back(object->id.index);
}

} // namespace knight
