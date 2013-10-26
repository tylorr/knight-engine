#ifndef KNIGHT_COMPONENT_MANAGER_H_
#define KNIGHT_COMPONENT_MANAGER_H_

#include "common.h"
#include "entity.h"
#include "component.h"

#include <unordered_map>

namespace knight {

class ComponentManager {
 public:
  ComponentManager() { }

  template<typename T>
  std::shared_ptr<T> AddComponent(const Entity::ID &, std::shared_ptr<T>);

  template<typename T, typename... Args>
  std::shared_ptr<T> AddComponent(const Entity::ID &, Args&&...);

  template<typename T>
  std::shared_ptr<T> GetComponent(const Entity::ID &);

  template<typename T>
  std::shared_ptr<T> RemoveComponent(const Entity::ID &);

  // void RemoveComponent(Entity *entity, const ComponentPtr &component);

 private:
  typedef std::shared_ptr<ComponentBase> BasePtr;
  typedef std::unordered_map<Entity::ID, BasePtr> EntityComponentMap;
  typedef std::unordered_map<ComponentBase::Family, EntityComponentMap> UniqueEntityComponentMap;
  typedef std::unordered_map<Entity::ID, ComponentMask> ComponentMaskMap;

  // component = [family][entity_id]
  UniqueEntityComponentMap entity_components_;
  ComponentMaskMap entity_component_mask_;

  KNIGHT_DISALLOW_COPY_AND_ASSIGN(ComponentManager);
};

template<typename T>
std::shared_ptr<T> ComponentManager::AddComponent(
    const Entity::ID &id,
    std::shared_ptr<T> component) {
  BasePtr base(std::static_pointer_cast<ComponentBase>(component));
  entity_components_[T::family()][id] = base;

  entity_component_mask_[id].set(T::family());

  return component;
}

template<typename T, typename... Args>
std::shared_ptr<T> ComponentManager::AddComponent(const Entity::ID &id,
                                                  Args&&... args) {
  return AddComponent<T>(id, std::make_shared<T>(std::forward(args)...));
}

template<typename T>
std::shared_ptr<T> ComponentManager::GetComponent(const Entity::ID &id) {
  return std::static_pointer_cast<T>(entity_components_[T::family()][id]);
}

template<typename T>
std::shared_ptr<T> ComponentManager::RemoveComponent(const Entity::ID &id) {
  std::shared_ptr<T> component;
  component = std::static_pointer_cast<T>(entity_components_[T::family()][id]);

  entity_components_[T::family()][id] = nullptr;

  entity_component_mask_[id].reset(T::family());

  return component;
}

} // namespace knight

#endif // KNIGHT_COMPONENT_MANAGER_H_
