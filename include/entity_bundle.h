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
  typedef std::shared_ptr<EntityBundle> ConstructorPtr;

  template<typename... Components>
  static ConstructorPtr Make(EntityManager *manager, Components&&... components) {
    ConstructorPtr constructor(new EntityBundle(manager));
    constructor->BindComponents(std::forward<Components>(components)...);
    return constructor;
  }

  Entity::ID Construct() {
    return bound_();
  }

 private:
  EntityBundle(EntityManager *manager) : manager_(manager) { }

  template<typename... Components>
  void BindComponents(Components&&... components) {
    bound_ = std::bind(&EntityBundle::CreateAndCompose<Components...>,
                       this,
                       std::forward<Components>(components)...);
  }

  template<typename... Components>
  Entity::ID CreateAndCompose(Components&&... components) {
    Entity::ID id = manager_->Create();
    Pass(AddComponent(id, std::forward<Components>(components))...);
    return id;
  }

  /// Noop func used to unpack CreateAndBind components... argument
  template<typename... Args>
  void Pass(Args&&... args) { }

  /// Return value is always ignored, used by Pass method
  template<typename T>
  bool AddComponent(const Entity::ID &id, const std::shared_ptr<T> &component) {
    manager_->AddComponent(id, component);
    return true;
  }

  std::function<Entity::ID()> bound_;
  EntityManager *manager_;
};

} // namespace knight

#endif // KNIGHT_ENTITY_BUNDLE_H_
