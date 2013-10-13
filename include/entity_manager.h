#ifndef ENTITY_MANAGER_H_
#define ENTITY_MANAGER_H_

#include "common.h"
#include "entity.h"

#include <vector>
#include <cstddef>

namespace knight {

class EntityManager {
 public:
  EntityManager() { }

  const size_t kChunkSize = 256;

  EntityID CreateEntity();
  Entity *GetEntity(const EntityID &id) const;
  void DestroyEntity(EntityID id);

 private:
  DISALLOW_COPY_AND_ASSIGN(EntityManager);

  std::vector<Entity *> entity_table_;
  std::vector<uint32_t> free_list_;
};

}; // namespace knight

#endif // ENTITY_MANAGER_H_
