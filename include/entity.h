#ifndef ENTITY_H_
#define ENTITY_H_

#include "common.h"

#include <cstdint>

namespace knight {

union EntityID {
  uint64_t id;
  struct {
    uint32_t index;
    uint32_t version;
  };

  operator uint64_t() const { return id; }
  EntityID &operator=(const uint64_t &val) { id = val; return *this; }
};

class Entity {
 public:
  Entity() : id_() { }

  EntityID id() const { return id_; }
  void set_id(EntityID id) { id_ = id; }

 private:
  DISALLOW_COPY_AND_ASSIGN(Entity);

  EntityID id_;
};

}; // namespace knight

#endif // ENTITY_H_
