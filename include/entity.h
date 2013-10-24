#ifndef KNIGHT_ENTITY_H_
#define KNIGHT_ENTITY_H_

#include "common.h"

#include <cstdint>

#include <memory>

namespace knight {

class Component;

class Entity {
 public:
  typedef std::shared_ptr<Component> ComponentPtr;

  Entity() : id_() { }

  ID id() const { return id_; }
  void set_id(ID id) { id_ = id; }

  unsigned int component_flags() const { return component_flags_; }

  void AddComponent(const ComponentPtr &flag);
  void RemoveComponent(const ComponentPtr &flag);
  bool HasComponent(const ComponentPtr &component);

 private:
  ID id_;
  unsigned int component_flags_;

  KNIGHT_DISALLOW_COPY_AND_ASSIGN(Entity);
};

} // namespace knight

#endif // KNIGHT_ENTITY_H_
