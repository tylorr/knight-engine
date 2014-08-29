#ifndef KNIGHT_ENTITY_BUNDLE_H_
#define KNIGHT_ENTITY_BUNDLE_H_

#include "entity.h"
#include "entity_manager.h"

#include <memory>

namespace knight {

/// Stores a collection of components that later can be added to a newly
/// created Entity
class EntityBundle {
 public:
  typedef std::shared_ptr<EntityBundle> BundlePtr;

  /// Components in this bundle will be shared between all entities constructed
  template<typename... Components>
  static BundlePtr MakeShared(EntityManager *manager,
                              Components&&... components);

  /// Components in this bundle are copied for each entity constructed
  template<typename... Components>
  static BundlePtr MakeCopy(EntityManager *manager,
                            Components&&... components);

  Entity::ID Construct() {
    return bound_();
  }

 private:
  EntityBundle(EntityManager *manager, const bool &copy)
    : manager_(manager), copy_(copy) { }

  template<typename... Components>
  void BindComponents(Components&&... components);

  template<typename... Components>
  Entity::ID Compose(Components&&... components);

  /// Noop func used to unpack Compose components... argument
  template<typename... Args>
  void Pass(Args&&... args) { }

  /// Return value is always ignored, used by Pass method
  template<typename T>
  bool AddComponent(const Entity::ID &id, std::shared_ptr<T> component);

  std::function<Entity::ID()> bound_;
  EntityManager *manager_;
  bool copy_;
};

template<typename... Components>
EntityBundle::BundlePtr EntityBundle::MakeShared(EntityManager *manager,
                                   Components&&... components) {
  BundlePtr bundle(new EntityBundle(manager, false));
  bundle->BindComponents(std::forward<Components>(components)...);
  return bundle;
}

template<typename... Components>
EntityBundle::BundlePtr EntityBundle::MakeCopy(EntityManager *manager,
                                   Components&&... components) {
  BundlePtr bundle(new EntityBundle(manager, true));
  bundle->BindComponents(std::forward<Components>(components)...);
  return bundle;
}

template<typename... Components>
void EntityBundle::BindComponents(Components&&... components) {
  bound_ = std::bind(&EntityBundle::Compose<Components...>, this,
                     std::forward<Components>(components)...);
}

template<typename... Components>
Entity::ID EntityBundle::Compose(Components&&... components) {
  Entity::ID id = manager_->Create();
  Pass(AddComponent(id, std::forward<Components>(components))...);
  return id;
}

template<typename T>
bool EntityBundle::AddComponent(const Entity::ID &id, std::shared_ptr<T> component) {
  if (copy_) {
    component.reset(new T(*component));
  }
  manager_->AddComponent(id, component);
  return true;
}

} // namespace knight

#endif // KNIGHT_ENTITY_BUNDLE_H_
