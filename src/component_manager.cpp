#include "component_manager.h"

namespace knight {

void ComponentManager::AddComponent(Entity *entity,
                                    const ComponentPtr &component) {
  if (component != nullptr) {
    component_map_[component->type()][entity->id()] = component;
    entity->AddComponent(component);
  }
}

void ComponentManager::RemoveComponent(Entity *entity,
                                       const ComponentPtr &component) {
  if (component != nullptr) {
    entity->RemoveComponent(component);
    component_map_[component->type()][entity->id()] = nullptr;
  }
}

} // namespace knight
