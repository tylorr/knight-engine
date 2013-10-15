#ifndef SLOT_MAP_H_
#define SLOT_MAP_H_

#include "common.h"

#include <logog.hpp>

#include <vector>
#include <cstddef>
#include <memory>

namespace knight {

template<typename T>
class SlotMap {
 public:
  const size_t kChunkSize = 256;

  SlotMap() { }

  ID Create() {
    // Are there no spare entities?
    if (free_list_.empty()) {
      // Create free entry for new entity in chunk
      for (int i = kChunkSize - 1; i >= 0; --i) {
        free_list_.push_back(entity_table_.size() * kChunkSize + i);
      }

      // Add new chunk to table
      entity_table_.push_back(std::unique_ptr<T[]>(new T[kChunkSize]));
    }

    uint32_t freeId = free_list_.back();
    free_list_.pop_back();

    // get first free entity
    T *entity = &entity_table_[freeId / kChunkSize][freeId % kChunkSize];

    // Update index
    ID id = entity->id();
    id.index = freeId;
    entity->set_id(id);

    // TODO: reset entity

    return id;
  }

  T *Get(const ID &id) const {
    uint32_t chunkIndex = id.index / kChunkSize;

    // Does the chunk exist?
    if (chunkIndex < entity_table_.size()) {
      T *entity = &entity_table_[chunkIndex][id.index % kChunkSize];

      // If there versions match return entity else NULL
      return entity->id() != id ? nullptr : entity;
    } else {

      // Chunk doesn't exist return NULL
      return nullptr;
    }
  }

  void Destroy(ID id) {
    T *entity = Get(id);

    if (entity != nullptr) {
      // Increment version to generate unique id
      id.version++;
      entity->set_id(id);
      free_list_.push_back(id.index);
    } else {
      ERR("Trying to delete non-existent entity: %lu", id.id);
    }
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(SlotMap);

  std::vector<std::unique_ptr<T[]>> entity_table_;
  std::vector<uint32_t> free_list_;
};

}; // namespace knight

#endif // SLOT_MAP_H_
