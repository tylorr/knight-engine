#ifndef KNIGHT_ENTITY_H_
#define KNIGHT_ENTITY_H_

#include "common.h"
#include "component.h"

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

  Entity() : id_(0), component_flags_(0) { }

  ID id() const { return id_; }

  // Internal use only
  void set_id(ID id) { id_ = id; }

  ComponentMask component_flags() const { return component_flags_; }

  void AddComponent(const ComponentBase::Family &);
  void RemoveComponent(const ComponentBase::Family &);
  bool HasComponent(const ComponentBase::Family &);

 private:
  ID id_;
  ComponentMask component_flags_;

  KNIGHT_DISALLOW_COPY_AND_ASSIGN(Entity);
};

} // namespace knight

namespace std {

// std::hash specialization for ID
template<>
struct hash<knight::Entity::ID> {
  size_t operator()(const knight::Entity::ID &id) const {
    return hash<uint64_t>()(id.id);
  }
};

}; // namespace std

#endif // KNIGHT_ENTITY_H_
