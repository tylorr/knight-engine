#pragma once

#include "common.h"

#include <vector>
#include <cstddef>
#include <memory>

namespace knight {

template<typename T, typename ID>
class SlotMap {
 public:
  const size_t kChunkSize = 256;

  SlotMap() { }
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

  std::vector<Chunk> slot_table_;
  std::vector<typename ID::type> free_list_;

  KNIGHT_DISALLOW_COPY_AND_ASSIGN(SlotMap);
};

template<typename T, typename ID>
ID SlotMap<T, ID>::Create() {
  // Are there no spare entities?
  if (free_list_.empty()) {
    free_list_.reserve(kChunkSize * (slot_table_.size() + 1));

    // Mark entire chunk as free
    typename ID::type last_index = slot_table_.size() * kChunkSize;
    for (int i = kChunkSize - 1; i >= 0; --i) {
      free_list_.push_back(last_index + i);
    }

    // Add new chunk to table
    slot_table_.emplace_back(new T[kChunkSize]);
  }

  // get first free index
  typename ID::type free_index = free_list_.back();
  free_list_.pop_back();

  // get object at index and update it's index
  T *object = &slot_table_[free_index / kChunkSize][free_index % kChunkSize];
  object->id_.index = free_index;

  return object->id_;
}

template<typename T, typename ID>
T *SlotMap<T, ID>::Get(const ID &id) const {
  typename ID::type chunkIndex = id.index / kChunkSize;

  // Does the chunk exist?
  if (chunkIndex < slot_table_.size()) {
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

  free_list_.push_back(object->id_.index);
}

} // namespace knight
