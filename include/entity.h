#pragma once

#include "common.h"

#include <cstdint>
#include <memory>

namespace knight {

template<typename T, typename ID>
class SlotMap;

class Entity {
 public:
  typedef ID64<Entity>::ID ID;

  ID id;
  
  Entity() : id(0) { }

 private:
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
