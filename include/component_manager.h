#ifndef KNIGHT_COMPONENT_MANAGER_H_
#define KNIGHT_COMPONENT_MANAGER_H_

#include "common.h"
#include "entity.h"
#include "component.h"

#include <unordered_map>

namespace knight {

class ComponentManager {
 public:
  typedef std::shared_ptr<Component> ComponentPtr;

  ComponentManager() { }

  template<typename T>
  std::shared_ptr<T> AddComponent(Entity *entity);

  void AddComponent(Entity *entity, const ComponentPtr &component);

  template<typename T>
  std::shared_ptr<T> GetComponent(const Entity *entity);

  template<typename T>
  void RemoveComponent(Entity *entity);

  void RemoveComponent(Entity *entity, const ComponentPtr &component);

 private:
  typedef std::unordered_map<ID, ComponentPtr> EntityComponentMap;
  typedef std::unordered_map<unsigned int, EntityComponentMap> ComponentTypeMap;

  // component = [component_type][entity_id]
  ComponentTypeMap component_map_;

  KNIGHT_DISALLOW_COPY_AND_ASSIGN(ComponentManager);
};

// Template implementations

template<typename T>
std::shared_ptr<T> ComponentManager::AddComponent(Entity *entity) {
  if (entity != nullptr) {
    std::shared_ptr<T> component(Component::Create<T>());
    AddComponent(entity, component);
    return component;
  } else {
    return nullptr;
  }
}

template<typename T>
std::shared_ptr<T> ComponentManager::GetComponent(const Entity *entity) {
  return std::static_pointer_cast<T>(
    component_map_[Component::TypeFor<T>()][entity->id()]
  );
}

template<typename T>
void ComponentManager::RemoveComponent(Entity *entity) {
  auto component = component_map_[Component::TypeFor<T>()][entity->id()];
  RemoveComponent(entity, component);
}

} // namespace knight

#endif // KNIGHT_COMPONENT_MANAGER_H_
