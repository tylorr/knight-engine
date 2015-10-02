#pragma once

#include "types.h"
#include "shader_types.h"
#include "component.h"
#include "pointers.h"

#include <collection_types.h>
#include <memory_types.h>

namespace knight {

class MeshComponent : public Component<MeshComponent> {
 public:
  struct InstanceData {
    Entity entity;
    Material *material;
    Mesh *mesh;
  };

  MeshComponent(foundation::Allocator &allocator);

  void Add(Entity e, Material &material, Mesh &mesh);
  void Destroy(uint32_t i);

  void Render() const;

  // void GC(const EntityManager &em);

 private:
  foundation::Array<InstanceData> data_;
};

} // namespace knight
