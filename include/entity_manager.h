#ifndef KNIGHT_ENTITY_MANAGER_H_
#define KNIGHT_ENTITY_MANAGER_H_

#include "common.h"
#include "entity.h"
#include "slot_map.h"

#include <map>

namespace knight {

class ComponentBase;

class EntityManager {
 public:
  EntityManager() { }

  Entity::ID Create();
  Entity *Get(const Entity::ID &);
  void Destroy(Entity::ID);

  ComponentMask component_mask(const Entity::ID &);

  template<typename T>
  void AddComponent(const Entity::ID &, std::shared_ptr<T>);

  /// Construct component with optional arguments and attach it
  /// to the entity.
  template<typename T, typename... Args>
  std::shared_ptr<T> AddComponent(const Entity::ID &, Args&&...);

  template<typename T>
  std::shared_ptr<T> GetComponent(const Entity::ID &);

  template<typename T>
  bool HasComponent(const Entity::ID &);

  template<typename T>
  void RemoveComponent(const Entity::ID &);

  /// Helper function that grabs the type from the passed in Component
  /// and forwards it to the other RemoveComponent function
  template<typename T>
  void RemoveComponent(const Entity::ID &, std::shared_ptr<T>);

 private:
  typedef std::shared_ptr<ComponentBase> BasePtr;
  typedef std::map<Entity::ID, BasePtr> EntityComponentMap;
  typedef std::map<unsigned int, EntityComponentMap> EntityComponentsMap;
  typedef std::map<Entity::ID, ComponentMask> ComponentMaskMap;

  // component = [family][entity_id]
  EntityComponentsMap entity_components_;
  ComponentMaskMap entity_component_mask_;
  SlotMap entities_;

  KNIGHT_DISALLOW_COPY_AND_ASSIGN(EntityManager);
};

template<typename T>
void EntityManager::AddComponent(const Entity::ID &id,
                                 std::shared_ptr<T> component) {
  BasePtr base(std::static_pointer_cast<ComponentBase>(component));
  entity_components_[T::family()][id] = base;
  entity_component_mask_[id].set(T::family());
}

template<typename T, typename... Args>
std::shared_ptr<T> EntityManager::AddComponent(const Entity::ID &id,
                                              Args&&... args) {
  auto component = std::make_shared<T>(std::forward<Args>(args)...);
  AddComponent<T>(id, component);
  return component;
}

template<typename T>
std::shared_ptr<T> EntityManager::GetComponent(const Entity::ID &id) {
  return std::static_pointer_cast<T>(entity_components_[T::family()][id]);
}

template<typename T>
bool EntityManager::HasComponent(const Entity::ID &id) {
  return entity_components_[T::family()][id] != nullptr;
  // return entity_component_mask_[id].test(T::family());
}

template<typename T>
void EntityManager::RemoveComponent(const Entity::ID &id) {
  entity_components_[T::family()][id] = nullptr;
  entity_component_mask_[id].reset(T::family());
}

template<typename T>
void EntityManager::RemoveComponent(const Entity::ID &, std::shared_ptr<T>) {
  RemoveComponent<T>();
}

} // namespace knight

#endif // KNIGHT_ENTITY_MANAGER_H_
