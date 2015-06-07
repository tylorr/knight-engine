#include "transform_component.h"
#include "random.h"
#include "entity_manager.h"

#include <array.h>
#include <logog.hpp>

using namespace foundation;

namespace knight {

TransformComponent::TransformComponent(foundation::Allocator &allocator) 
    : Component{allocator},
      data_{},
      allocator_{allocator} { }

TransformComponent::~TransformComponent() {
  allocator_.deallocate(data_.buffer);
}

// void TransformComponent::Add(Entity e, Material &material, VertexArray &vao, uint32_t index_count) {
//   if (data_.size + 1 >= data_.capacity) {
//     Allocate((data_.capacity + 1) * 2);
//   }
  
//   auto index = data_.size;
//   data_.entity[index] = e;
//   data_.material[index] = &material;
//   data_.vao[index] = vao.handle();
//   data_.index_count[index] = index_count;

//   hash::set(map_, e.id, index);
//   ++data_.size;
// }

// void TransformComponent::Render() const {
//   for (auto i = 0u; i < data_.size; ++i) {
//     data_.material[i]->Bind();
//     GL(glBindVertexArray(data_.vao[i]));
//     GL(glDrawElements(GL_TRIANGLES, data_.index_count[i], GL_UNSIGNED_INT, nullptr));
//   }
// }

void TransformComponent::Allocate(uint32_t size) {
  XASSERT(size > data_.size, "Cannot allocate smaller amount");

  InstanceData new_data;
  const auto bytes = size * (sizeof(Entity) + sizeof(glm::mat4) + 
                             sizeof(glm::mat4) + (4 * sizeof(Instance)));

  new_data.buffer = allocator_.allocate(bytes);
  new_data.size = data_.size;
  new_data.capacity = size;

  new_data.entity = static_cast<Entity *>(new_data.buffer);
  new_data.local = reinterpret_cast<glm::mat4 *>(new_data.entity + size);
  new_data.world = new_data.local + size;
  new_data.parent = reinterpret_cast<Instance *>(new_data.world + size);
  new_data.first_child = new_data.parent + size;
  new_data.next_sibling = new_data.first_child + size;
  new_data.prev_sibling = new_data.next_sibling + size;

  memcpy(new_data.entity, data_.entity, data_.size * sizeof(Entity));
  memcpy(new_data.local, data_.local, data_.size * sizeof(glm::mat4));
  memcpy(new_data.world, data_.world, data_.size * sizeof(glm::mat4));
  memcpy(new_data.parent, data_.parent, data_.size * sizeof(Instance));
  memcpy(new_data.first_child, data_.first_child, data_.size * sizeof(Instance));
  memcpy(new_data.next_sibling, data_.next_sibling, data_.size * sizeof(Instance));
  memcpy(new_data.prev_sibling, data_.prev_sibling, data_.size * sizeof(Instance));

  allocator_.deallocate(data_.buffer);
  data_ = new_data;
}

void TransformComponent::Destroy(uint32_t i) {
  // auto last = data_.size - 1;
  // auto entity = data_.entity[i];
  // auto last_entity = data_.entity[last];

  // data_.entity[i] = data_.entity[last];
  // data_.material[i] = data_.material[last];
  // data_.vao[i] = data_.vao[last];
  // data_.index_count[i] = data_.index_count[last];

  // hash::set(map_, last_entity.id, i);
  // hash::remove(map_, entity.id);

  // --data_.size;
}

void TransformComponent::GC(const EntityManager &em) {
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

bool TransformComponent::IsValid(Instance i) {
  // treat index 0 as a null value
  return i.i > 0 && i.i < data_.size;
}

void TransformComponent::set_local(Instance i, const glm::mat4 &m) {
  data_.local[i.i] = m;
  auto parent = data_.parent[i.i];
  auto parent_tm = IsValid(parent) ? data_.world[parent.i] : glm::mat4(1.0);
  Transform(i, parent_tm);
}

void TransformComponent::Transform(Instance i, const glm::mat4 &p) {
  Array<Instance> transforms{allocator_};

  data_.world[i.i] = data_.local[i.i] * p;
  GetChildren(i, transforms);

  while (!array::empty(transforms)) {
    auto transform = array::back(transforms);
    array::pop_back(transforms);

    auto parent = data_.parent[transform.i];
    data_.world[transform.i] = data_.local[transform.i] * data_.world[parent.i];

    GetChildren(transform, transforms);
  }
}

void TransformComponent::GetChildren(Instance i, 
    foundation::Array<Instance> &children) {
  auto child = data_.first_child[i.i];
  while (IsValid(child)) {
    array::push_back(children, child);
    child = data_.next_sibling[child.i];
  }
}

} // namespace knight
