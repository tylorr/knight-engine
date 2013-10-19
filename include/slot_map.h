#ifndef SLOT_MAP_H_
#define SLOT_MAP_H_

#include "common.h"

#include <logog.hpp>

#include <vector>
#include <cstddef>
#include <memory>

namespace knight {

template<typename T>
class SlotMap {
 public:
  typedef std::unique_ptr<T[]> Chunk;

  const size_t kChunkSize = 256;

  SlotMap() { }

  ID Create();
  T *Get(const ID &id) const;
  void Destroy(ID id);

 private:
  std::vector<Chunk> slot_table_;
  std::vector<uint32_t> free_list_;

  KNIGHT_DISALLOW_COPY_AND_ASSIGN(SlotMap);
};

}; // namespace knight

#include "slot_map.tpp"

#endif // SLOT_MAP_H_
