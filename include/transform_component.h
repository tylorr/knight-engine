#pragma once

#include "types.h"
#include "component.h"
#include "pointers.h"

#include <collection_types.h>
#include <memory_types.h>

#include <glm/glm.hpp>

namespace knight {

namespace transform {

glm::mat4 GetRelative(const glm::mat4 &target, const glm::mat4 &transform);

}

class TransformComponent : public Component<TransformComponent> {
 public:
  struct InstanceData {
    uint32_t size;
    uint32_t capacity;
    Pointer<char[]> buffer;

    Entity *entity;
    glm::mat4 *local;
    glm::mat4 *world;
    Instance *parent;
    Instance *first_child;
    Instance *next_sibling;
    Instance *prev_sibling;
  };

  TransformComponent(foundation::Allocator &allocator);

  void add(Entity e);
  void add(Entity e, Instance parent);

  void allocate(uint32_t size);
  void destroy(uint32_t i);

  void collect_garbage(const EntityManager &em);

  bool is_valid(Instance instance) const;
  
  void swap(Instance instanceA, Instance instanceB);

  uint32_t capacity() const { return data_.capacity; }

  void set_local(Instance instance, const glm::mat4 &local);
  glm::mat4 local(Instance instance) const;

  glm::mat4 world(Instance instance) const;

  void set_parent(Instance instance, Instance parent);

  void transform(Instance instance, const glm::mat4 &parent);

 private:
  foundation::Allocator &allocator_;
  InstanceData data_;
};

} // namespace knight
