#pragma once

#include "common.h"

#include <cstdint>
#include <memory>

namespace knight {

template<typename T>
class SlotMap;

class Entity {
 public:
  typedef ID<Entity> ID;

  Entity() : id_(0) { }

  ID id() const { return id_; }

 private:
  friend class SlotMap<Entity>;

  ID id_;

  KNIGHT_DISALLOW_COPY_AND_ASSIGN(Entity);
};

} // namespace knight

namespace std {

// std::hash specialization for Entity::ID
template<>
struct hash<knight::Entity::ID> {
  size_t operator()(const knight::Entity::ID &id) const {
    return hash<uint64_t>()(id.id);
  }
};

} // namespace std
