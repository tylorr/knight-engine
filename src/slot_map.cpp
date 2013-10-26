#include "slot_map.h"

#include <logog.hpp>

namespace knight {

Entity::ID SlotMap::Create() {
  // Are there no spare entities?
  if (free_list_.empty()) {
    free_list_.reserve(kChunkSize * (slot_table_.size() + 1));

    // Mark entire chunk as free
    uint32_t last_index = slot_table_.size() * kChunkSize;
    for (int i = kChunkSize - 1; i >= 0; --i) {
      free_list_.push_back(last_index + i);
    }

    // Add new chunk to table
    slot_table_.push_back(Chunk(new Entity[kChunkSize]));
  }

  uint32_t free_index = free_list_.back();
  free_list_.pop_back();

  // get first free object
  Entity *entity = &slot_table_[free_index / kChunkSize][free_index % kChunkSize];
  entity->id_.index = free_index;

  return entity->id_;
}

Entity *SlotMap::Get(const Entity::ID &id) const {
  uint32_t chunkIndex = id.index / kChunkSize;

  // Does the chunk exist?
  if (chunkIndex < slot_table_.size()) {
    Entity *entity = &slot_table_[chunkIndex][id.index % kChunkSize];

    // If the ids (specifically the versions) match return object else NULL
    return entity->id() != id ? nullptr : entity;
  } else {

    // Chunk doesn't exist return NULL
    return nullptr;
  }
}

void SlotMap::Destroy(Entity::ID id) {
  Entity *entity = Get(id);

  if (entity != nullptr) {
    // Increment version to generate unique id and invalidate old id
    entity->id_.version++;

    free_list_.push_back(entity->id_.index);
  } else {
    ERR("Trying to delete non-existent object: %lu", id.id);
  }
}

} // namespace knight
