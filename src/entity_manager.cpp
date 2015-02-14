#include "entity_manager.h"

#include <logog.hpp>

namespace knight {

Entity::ID EntityManager::Create() {
  return entities_.Create();
}

Entity *EntityManager::Get(Entity::ID id) const {
  return entities_.Get(id);
}

void EntityManager::Destroy(Entity::ID id) {
  XASSERT(Alive(id), "Cannot destroy non-existent Entity");
  entities_.Destroy(id);
}

bool EntityManager::Alive(Entity::ID id) const {
  return Get(id) != nullptr;
}

bool EntityManager::Alive(const Entity &e) const {
  return Alive(e.id);
}

} // namespace knight
