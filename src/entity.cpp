#include "entity.h"
#include "component.h"

namespace knight {

void Entity::AddComponent(const ComponentPtr &component) {
  if (component != nullptr) {
    component_flags_ |= component->flag();
  }
}

void Entity::RemoveComponent(const ComponentPtr &component) {
  if (component != nullptr) {
    component_flags_ &= ~component->flag();
  }
}

bool Entity::HasComponent(const ComponentPtr &component) {
  bool result = false;

  if (component != nullptr) {
    result = (component_flags_ & component->flag()) != 0;
  }

  return result;
}

} // namespace knight
