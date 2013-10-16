#ifndef COMPONENT_MANAGER_H_
#define COMPONENT_MANAGER_H_

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
  std::shared_ptr<T> AddComponent(Entity &entity) {
    std::shared_ptr<T> component(Component::Create<T>());
    AddComponent(entity, component);
    return component;
  }

  void AddComponent(Entity &entity, const ComponentPtr &component) {
    if (component != nullptr) {
      component_map_[component->type()][entity.id()] = component;
      entity.AddComponent(component);
    }
  }

  template<typename T>
  std::shared_ptr<T> GetComponent(const Entity &entity) {
    return std::static_pointer_cast<T>(component_map_[Component::TypeFor<T>()][entity.id()]);
  }

  template<typename T>
  void RemoveComponent(Entity &entity) {
    auto component = component_map_[Component::TypeFor<T>()][entity.id()];
    RemoveComponent(entity, component);
  }

  void RemoveComponent(Entity &entity, const ComponentPtr &component) {
    if (component != nullptr) {
      entity.RemoveComponent(component);
      component_map_[component->type()][entity.id()] = nullptr;
    }
  }

 private:
  typedef std::unordered_map<ID, ComponentPtr> EntityComponentMap;
  typedef std::unordered_map<unsigned int, EntityComponentMap> ComponentTypeMap;

  // component = [component_type][entity_id]
  ComponentTypeMap component_map_;

  KNIGHT_DISALLOW_COPY_AND_ASSIGN(ComponentManager);
};

}; // namespace knight

#endif // COMPONENT_MANAGER_H_
