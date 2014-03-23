#include "entity_manager.h"

#include <logog.hpp>

namespace knight {

Entity::ID EntityManager::Create() {
  return entities_.Create();
}

Entity *EntityManager::Get(const Entity::ID &id) {
  return entities_.Get(id);
}

ComponentMask EntityManager::component_mask(const Entity::ID &id) {
  return entity_component_mask_[id];
}

void EntityManager::Destroy(Entity::ID id) {
  // Does this entity exist?
  if (Get(id) == nullptr) {
    ERR("Trying to destroy non-existent Entity");
    return;
  }

  for (auto &components: entity_components_) {
    components.second[id] = nullptr;
  }
  entity_component_mask_[id].reset();

  entities_.Destroy(id);
}


} // namespace knight
