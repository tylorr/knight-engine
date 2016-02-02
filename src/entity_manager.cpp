#include "entity_manager.h"

#include <logog.hpp>

namespace knight {

Entity::ID EntityManager::create() {
  return entities_.create();
}

Entity *EntityManager::get(Entity::ID id) const {
  return entities_.get(id);
}

void EntityManager::destroy(Entity::ID id) {
  XASSERT(alive(id), "Cannot destroy non-existent Entity");
  entities_.destroy(id);
}

bool EntityManager::alive(Entity::ID id) const {
  return get(id) != nullptr;
}

bool EntityManager::alive(const Entity &e) const {
  return alive(e.id);
}

} // namespace knight
