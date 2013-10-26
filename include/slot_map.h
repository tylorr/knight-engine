#ifndef KNIGHT_SLOT_MAP_H_
#define KNIGHT_SLOT_MAP_H_

#include "entity.h"

#include <vector>
#include <cstddef>
#include <memory>

namespace knight {


class SlotMap {
 public:
  const size_t kChunkSize = 256;

  SlotMap() { }

  Entity::ID Create();
  Entity *Get(const Entity::ID &id) const;
  void Destroy(Entity::ID id);

 private:
  typedef std::unique_ptr<Entity[]> Chunk;

  std::vector<Chunk> slot_table_;
  std::vector<uint32_t> free_list_;

  KNIGHT_DISALLOW_COPY_AND_ASSIGN(SlotMap);
};

} // namespace knight

#endif // KNIGHT_SLOT_MAP_H_
