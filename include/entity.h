#ifndef ENTITY_H_
#define ENTITY_H_

#include "common.h"

#include <cstdint>

namespace knight {

class Entity {
 public:
  Entity() : id_() { }

  ID id() const { return id_; }
  void set_id(ID id) { id_ = id; }

 private:
  ID id_;

  KNIGHT_DISALLOW_COPY_AND_ASSIGN(Entity);
};

}; // namespace knight

#endif // ENTITY_H_
