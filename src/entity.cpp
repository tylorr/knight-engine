#include "entity.h"

namespace knight {

void Entity::AddComponent(const ComponentBase::Family &family) {
  // component_flags_.set(family);
}

void Entity::RemoveComponent(const ComponentBase::Family &family) {
  // component_flags_.reset(family);
}

bool Entity::HasComponent(const ComponentBase::Family &family) {
  // return component_flags_.test(family);
  return false;
}

} // namespace knight
