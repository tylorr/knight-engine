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
  std::shared_ptr<T> AddComponent(Entity &entity);

  void AddComponent(Entity &entity, const ComponentPtr &component);

  template<typename T>
  std::shared_ptr<T> GetComponent(const Entity &entity);

  template<typename T>
  void RemoveComponent(Entity &entity);

  void RemoveComponent(Entity &entity, const ComponentPtr &component);

 private:
  typedef std::unordered_map<ID, ComponentPtr> EntityComponentMap;
  typedef std::unordered_map<unsigned int, EntityComponentMap> ComponentTypeMap;

  // component = [component_type][entity_id]
  ComponentTypeMap component_map_;

  KNIGHT_DISALLOW_COPY_AND_ASSIGN(ComponentManager);
};

}; // namespace knight

#include "component_manager.tpp"

#endif // COMPONENT_MANAGER_H_
