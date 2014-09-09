#pragma once

#include "common.h"

#include <cstdint>
#include <memory>

namespace knight {

class Entity {
 public:
  union ID {
    uint64_t id;
    struct {
      uint32_t index;
      uint32_t version;
    };

    ID() : id(0) { }
    ID(const uint64_t &val) : id(val) { }
    ID(uint64_t &&val) : id(val) { }

    operator uint64_t() const { return id; }
    ID &operator=(const uint64_t &val) {
      id = val;
      return *this;
    }
  };

  Entity() : id_(0) { }

  ID id() const { return id_; }

 private:
  friend class SlotMap;

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

}; // namespace std
