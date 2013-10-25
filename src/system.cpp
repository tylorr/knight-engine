#include "system.h"
#include "entity.h"

namespace knight {

void System::OnEntityCreated(const Entity *entity) {

  // Does the entity have proper componenets?
  if (entity != nullptr &&
      (component_flags_ & entity->component_flags()) == component_flags_) {
    entities_.emplace(entity->id());
  }
}

void System::OnEntityDestroyed(const Entity *entity) {
  if (entity != nullptr) {
    entities_.erase(entity->id());
  }
}

} // knight
