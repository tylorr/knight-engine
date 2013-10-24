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

  Entity() : id_(0), component_flags_(0) { }

  ID id() const { return id_; }
  void set_id(ID id) { id_ = id; }

  ComponentFlag component_flags() const { return component_flags_; }

  void AddComponent(const ComponentPtr &flag);
  void RemoveComponent(const ComponentPtr &flag);
  bool HasComponent(const ComponentPtr &component);

 private:
  ID id_;
  ComponentFlag component_flags_;

  KNIGHT_DISALLOW_COPY_AND_ASSIGN(Entity);
};

} // namespace knight

#endif // KNIGHT_ENTITY_H_
