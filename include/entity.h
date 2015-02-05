#pragma once

#include "common.h"
#include "types.h"

#include <cstdint>
#include <memory>

namespace knight {

class Entity {
 public:
  using ID = ID64<Entity>::ID;
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
