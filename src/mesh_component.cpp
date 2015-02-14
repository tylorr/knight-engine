#include "mesh_component.h"
#include "entity_manager.h"
#include "random.h"
#include "material.h"

#include <hash.h>
#include <logog.hpp>

using namespace foundation;

namespace knight {

MeshComponent::MeshComponent(foundation::Allocator &allocator) 
    : data_{},
      allocator_{allocator},
      map_{allocator} { }

MeshComponent::~MeshComponent() {
  allocator_.deallocate(data_.buffer);
}

auto MeshComponent::MakeInstance(int i) -> Instance {
  return Instance{i};
}

auto MeshComponent::Lookup(Entity e) -> Instance {
  return MakeInstance(hash::get(map_, e.id, 0u));
}

void MeshComponent::Add(Entity e, Material &material, VertexArray &vao, uint32_t index_count) {
  if (data_.size + 1 >= data_.capacity) {
    Allocate((data_.capacity + 1) * 2);
  }
  
  auto index = data_.size;
  data_.entity[index] = e;
  data_.material[index] = &material;
  data_.vao[index] = vao.handle();
  data_.index_count[index] = index_count;

  hash::set(map_, e.id, index);
  ++data_.size;
}

void MeshComponent::Render() const {
  for (auto i = 0u; i < data_.size; ++i) {
    data_.material[i]->Bind();
    GL(glBindVertexArray(data_.vao[i]));
    GL(glDrawElements(GL_TRIANGLES, data_.index_count[i], GL_UNSIGNED_INT, nullptr));
  }
}

void MeshComponent::Allocate(uint32_t size) {
  XASSERT(size > data_.size, "Cannot allocate smaller amount");

  InstanceData new_data;
  const auto bytes = size * (sizeof(Entity) + sizeof(Material *) + 
                             sizeof(GLuint) + sizeof(uint32_t));

  new_data.buffer = allocator_.allocate(bytes);
  new_data.size = data_.size;
  new_data.capacity = size;

  new_data.entity = static_cast<Entity *>(new_data.buffer);
  new_data.material = reinterpret_cast<Material **>(new_data.entity + size);
  new_data.vao = reinterpret_cast<GLuint *>(new_data.material + size);
  new_data.index_count = reinterpret_cast<uint32_t *>(new_data.vao + size);

  memcpy(new_data.entity, data_.entity, data_.size * sizeof(Entity));
  memcpy(new_data.material, data_.material, data_.size * sizeof(Material *));
  memcpy(new_data.vao, data_.vao, data_.size * sizeof(GLuint));
  memcpy(new_data.index_count, data_.index_count, data_.size * sizeof(uint32_t));

  allocator_.deallocate(data_.buffer);
  data_ = new_data;
}

void MeshComponent::Destroy(uint32_t i) {
  auto last = data_.size - 1;
  auto entity = data_.entity[i];
  auto last_entity = data_.entity[last];

  data_.entity[i] = data_.entity[last];
  data_.material[i] = data_.material[last];
  data_.vao[i] = data_.vao[last];
  data_.index_count[i] = data_.index_count[last];

  hash::set(map_, last_entity.id, i);
  hash::remove(map_, entity.id);

  --data_.size;
}

void MeshComponent::GC(const EntityManager &em) {
  const auto kAliveInARowThreshold = 4u;
  auto alive_in_row = 0u;
  while (data_.size > 0 && alive_in_row < kAliveInARowThreshold) {
    auto i = random_in_range(0u, data_.size - 1u);
    if (em.Alive(data_.entity[i])) {
      ++alive_in_row;
      continue;
    }
    alive_in_row = 0;
    Destroy(i);
  }
}

} // namespace knight
