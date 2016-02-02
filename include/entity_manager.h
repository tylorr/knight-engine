#pragma once

#include "common.h"
#include "types.h"
#include "slot_map.h"

#include <memory_types.h>

namespace knight {

class EntityManager {
 public:
  EntityManager(foundation::Allocator &allocator) : entities_{allocator} { }

  Entity::ID create();
  Entity *get(Entity::ID id) const;
  void destroy(Entity::ID id);

  bool alive(Entity::ID id) const;
  bool alive(const Entity &e) const;

 private:
  SlotMap<Entity, Entity::ID> entities_;

  KNIGHT_DISALLOW_COPY_AND_ASSIGN(EntityManager);
};

} // namespace knight
