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
  std::shared_ptr<T> AddComponent(const Entity &entity) {
    std::shared_ptr<T> component(Component::Create<T>());
    AddComponent(entity, component);
    return component;
  }

  void AddComponent(const Entity &entity, const ComponentPtr &component) {
    unsigned int componentType = component->type();
    component_map_[componentType][entity.id()] = component;

    // TODO: add type to entity list
  }

  template<typename T>
  std::shared_ptr<T> GetComponent(const Entity &entity) {
    return std::static_pointer_cast<T>(component_map_[Component::TypeFor<T>()][entity.id()]);
  }

  template<typename T>
  void RemoveComponent(const Entity &entity) {
    unsigned int componentType = Component::TypeFor<T>();
    component_map_[componentType][entity.id()] = nullptr;

    // TODO: remove type from entity list
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
