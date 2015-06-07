#pragma once

#include "types.h"
#include "component.h"

#include <collection_types.h>
#include <memory_types.h>

#include <glm/glm.hpp>

namespace knight {

class TransformComponent : public Component<TransformComponent> {
 public:
  struct InstanceData {
    uint32_t size;
    uint32_t capacity;
    void *buffer;

    Entity *entity;
    glm::mat4 *local;
    glm::mat4 *world;
    Instance *parent;
    Instance *first_child;
    Instance *next_sibling;
    Instance *prev_sibling;
  };

  TransformComponent(foundation::Allocator &allocator);
  ~TransformComponent();

  void Add(Entity e);

  void Allocate(uint32_t size);
  void Destroy(uint32_t i);

  void GC(const EntityManager &em);

  void GetChildren(Instance i, foundation::Array<Instance> &children);

  bool IsValid(Instance i);

  void set_local(Instance i, const glm::mat4 &m);
  void Transform(Instance i, const glm::mat4 &parent);

 private:
  InstanceData data_;
  foundation::Allocator &allocator_;
};

} // namespace knight
