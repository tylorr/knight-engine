#ifndef KNIGHT_COMPONENT_MANAGER_H_
#define KNIGHT_COMPONENT_MANAGER_H_

#include "common.h"
#include "entity.h"
#include "component.h"

#include <unordered_map>

namespace knight {

class ComponentManager {
 public:
  // typedef std::shared_ptr<Component> ComponentPtr;

  ComponentManager() { }

  template<typename T>
  std::shared_ptr<T> AddComponent(Entity *entity, std::shared_ptr<T> component);

  template<typename T, typename... Args>
  std::shared_ptr<T> AddComponent(Entity *entity, Args&&... args);

  template<typename T>
  std::shared_ptr<T> GetComponent(const Entity *entity);

  template<typename T>
  std::shared_ptr<T> RemoveComponent(Entity *entity);

  // void RemoveComponent(Entity *entity, const ComponentPtr &component);

 private:
  typedef std::shared_ptr<ComponentBase> BasePtr;
  typedef std::unordered_map<Entity::ID, BasePtr> EntityComponentMap;
  typedef std::unordered_map<unsigned int, EntityComponentMap> ComponentTypeMap;

  // component = [family][entity_id]
  ComponentTypeMap component_map_;

  KNIGHT_DISALLOW_COPY_AND_ASSIGN(ComponentManager);
};

template<typename T>
std::shared_ptr<T> ComponentManager::AddComponent(Entity *entity,
                                                  std::shared_ptr<T> component) {
  BasePtr base(std::static_pointer_cast<ComponentBase>(component));
  component_map_[T::family()][entity->id()] = base;
  entity->AddComponent(T::family());

  return component;
}

template<typename T, typename... Args>
std::shared_ptr<T> ComponentManager::AddComponent(Entity *entity, Args&&... args) {
  return AddComponent<T>(entity, std::shared_ptr<T>(new T(std::forward<Args>(args)...)));
}

// template<typename T>
// std::shared_ptr<T> ComponentManager::AddComponent(Entity *entity) {
//   if (entity != nullptr) {
//     std::shared_ptr<T> component(Component::Create<T>());
//     AddComponent(entity, component);
//     return component;
//   } else {
//     return nullptr;
//   }
// }

template<typename T>
std::shared_ptr<T> ComponentManager::GetComponent(const Entity *entity) {
  if (entity != nullptr) {
    return std::static_pointer_cast<T>(
      component_map_[T::family()][entity->id()]
    );
  } else {
    return nullptr;
  }
}

template<typename T>
std::shared_ptr<T> ComponentManager::RemoveComponent(Entity *entity) {
  if (entity != nullptr) {
    std::shared_ptr<T> component(std::static_pointer_cast<T>(component_map_[T::family()][entity->id()]));
    component_map_[T::family()][entity->id()] = nullptr;
    entity->RemoveComponent(T::family());
    return component;
  } else {
    return nullptr;
  }
}

// template<typename T>
// void ComponentManager::RemoveComponent(Entity *entity) {
//   auto component = component_map_[Component::TypeFor<T>()][entity->id()];
//   RemoveComponent(entity, component);
// }

} // namespace knight

#endif // KNIGHT_COMPONENT_MANAGER_H_
