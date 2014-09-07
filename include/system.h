#ifndef KNIGHT_SYSTEM_H_
#define KNIGHT_SYSTEM_H_

#include "common.h"
#include "entity.h"

#include <unordered_set>

namespace knight {

class Entity;

class System {
 public:
  System() : component_flags_(0) { }
  virtual ~System() { }

  // template<typename T>
  // void SetFlag();

  virtual void OnEntityCreated(const Entity *);
  virtual void OnEntityDestroyed(const Entity *);

  size_t size() const { return entities_.size(); }
  ComponentMask component_flags() const { return component_flags_; }

 private:
  ComponentMask component_flags_;
  std::unordered_set<Entity::ID> entities_;
};

// template<typename T>
// void System::SetFlag() {
//   component_flags_ |= Component::TypeFor<T>();
// }

} // namespace knight

#endif // KNIGHT_SYSTEM_H_
