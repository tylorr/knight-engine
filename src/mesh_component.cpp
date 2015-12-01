#include "mesh_component.h"
#include "entity_manager.h"
#include "random.h"
#include "material.h"
#include "pointers.h"
#include "array_object.h"
#include "iterators.h"
#include "array.h"

#include <hash.h>
#include <logog.hpp>

using namespace foundation;

namespace knight {

MeshComponent::MeshComponent(foundation::Allocator &allocator) :
  Component{allocator},
  data_{allocator} {}

void MeshComponent::Add(Entity e, Material &material, ArrayObject &vao) {
  auto index = array::size(data_);
  array::push_back(data_, {e, &material, &vao });
  hash::set(map_, e.id, index);
}

void MeshComponent::Destroy(uint32_t i) {
  auto last = array::size(data_) - 1;
  auto entity = data_[i].entity;
  auto last_entity = data_[last].entity;

  data_[i] = data_[last];

  hash::set(map_, last_entity.id, i);
  hash::remove(map_, entity.id);

  array::pop_back(data_);
}

void MeshComponent::Render() const {
  for (auto &&instance : data_) {
    instance.material->Bind();
    instance.vao->Draw();
  }
}

// void MeshComponent::GC(const EntityManager &em) {
//   const auto kAliveInARowThreshold = 4u;
//   auto alive_in_row = 0u;
//   while (data_.size > 0 && alive_in_row < kAliveInARowThreshold) {
//     auto i = random_in_range(0u, data_.size - 1u);
//     if (em.Alive(data_.entity[i])) {
//       ++alive_in_row;
//       continue;
//     }
//     alive_in_row = 0;
//     Destroy(i);
//   }
// }

} // namespace knight
