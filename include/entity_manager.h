#pragma once

#include "common.h"
#include "types.h"
#include "slot_map.h"

#include <memory_types.h>

namespace knight {

class EntityManager {
 public:
  EntityManager(foundation::Allocator &allocator) : entities_{allocator} { }

  Entity::ID Create();
  Entity *Get(Entity::ID id) const;
  void Destroy(Entity::ID id);

  bool Alive(Entity::ID id) const;
  bool Alive(const Entity &e) const;

 private:
  SlotMap<Entity, Entity::ID> entities_;

  KNIGHT_DISALLOW_COPY_AND_ASSIGN(EntityManager);
};

} // namespace knight
