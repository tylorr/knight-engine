#pragma once

#include "types.h"
#include "shader_types.h"
#include "component.h"

#include <collection_types.h>
#include <memory_types.h>

namespace knight {

class MeshComponent : public Component<MeshComponent> {
 public:
  struct InstanceData {
    uint32_t size;
    uint32_t capacity;
    void *buffer;

    Entity *entity;
    Material **material;
    GLuint *vao;
    uint32_t *index_count;
  };

  MeshComponent(foundation::Allocator &allocator);
  ~MeshComponent();

  void Add(Entity e, Material &material, VertexArray &vao, uint32_t index_count);

  void Render() const;

  void Allocate(uint32_t size);
  void Destroy(uint32_t i);

  void GC(const EntityManager &em);

 private:
  InstanceData data_;
  foundation::Allocator &allocator_;
};

} // namespace knight
