#pragma once

#include "types.h"
#include "shader_types.h"
#include "component.h"
#include "pointers.h"
#include "vector.h"

#include <memory_types.h>

namespace knight {

class MeshComponent : public Component<MeshComponent> {
 public:
  struct InstanceData {
    Entity entity;
    Material *material;
    ArrayObject *vao;
  };

  MeshComponent(foundation::Allocator &allocator);

  void add(Entity e, Material &material, ArrayObject &vao);
  void destroy(uint32_t i);

  void render() const;

  // void GC(const EntityManager &em);

 private:
  Vector<InstanceData> data_;
};

} // namespace knight
