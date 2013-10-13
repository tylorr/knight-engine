#include "entity_manager.h"

#include <logog.hpp>

namespace knight {

EntityID EntityManager::CreateEntity() {
  // Are there no spare entities?
  if (free_list_.empty()) {
    // Create new entity chunk
    Entity *chunk = new Entity[kChunkSize];

    // Create free entry for new entity in chunk
    for (int i = kChunkSize - 1; i >= 0; --i) {
      free_list_.push_back(entity_table_.size() * kChunkSize + i);
    }

    // add the chunk to the entity table
    entity_table_.push_back(chunk);
  }

  uint32_t freeId = free_list_.back();
  free_list_.pop_back();

  // get first free entity
  Entity *entity = entity_table_[freeId / kChunkSize] + (freeId % kChunkSize);

  // Update index
  // Note: version of new Entities is not guaranteed to be 0
  EntityID id = entity->id();
  id.index = freeId;
  entity->set_id(id);

  // TODO: reset entity

  return id;
}

Entity *EntityManager::GetEntity(const EntityID &id) const {
  Entity *entity = entity_table_[id.index / kChunkSize] + (id.index % kChunkSize);

  // return null if ids do not match (aka versions do not match)
  return entity->id() != id ? nullptr : entity;
}

void EntityManager::DestroyEntity(EntityID id) {
  Entity *entity = GetEntity(id);

  if (entity) {
    // DBUG("Got to here");
    id.version++;
    entity->set_id(id);
    free_list_.push_back(id.index);
  } else {
    WARN("Trying to delete entity that does not exist using ID: %lu", id.id);
  }
}

}; // namespace knight
