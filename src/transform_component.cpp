#include "transform_component.h"
#include "random.h"
#include "entity_manager.h"
#include "memory_block.h"

#include <array.h>
#include <logog.hpp>

using namespace foundation;

namespace knight {

namespace transform {

glm::mat4 get_relative(const glm::mat4 &target, const glm::mat4 &transform) {
  return transform * glm::inverse(target);
}

}

TransformComponent::TransformComponent(foundation::Allocator &allocator) :
    Component{allocator},
    allocator_{allocator},
    data_{} {}

void TransformComponent::add(Entity e) {
  if (data_.size + 1 >= data_.capacity) {
    // TODO: Find best growth strategy
    allocate(data_.capacity * 2 + 8);
  }

  auto null_instance = make_instance(-1);
  auto index = data_.size;
  data_.entity[index] = e;
  data_.local[index] = glm::mat4(1.0f);
  data_.world[index] = glm::mat4(1.0f);
  data_.parent[index] = null_instance;
  data_.first_child[index] = null_instance;
  data_.next_sibling[index] = null_instance;
  data_.prev_sibling[index] = null_instance;

  hash::set(map_, e.id, index);
  ++data_.size;
}

void TransformComponent::add(Entity e, Instance parent) {
  add(e);
  set_parent(lookup(e), parent);
}

void TransformComponent::allocate(uint32_t capacity) {
  data_.buffer = 
    memory_block::grow_contiguous(
      allocator_,
      data_.capacity, capacity,
      data_.entity,
      data_.local,
      data_.world,
      data_.parent,
      data_.first_child,
      data_.next_sibling,
      data_.prev_sibling);
  data_.capacity = capacity;
}

void TransformComponent::destroy(uint32_t i) {
  auto last = data_.size - 1;

  auto instance = make_instance(i);
  auto last_instance = make_instance(last);

  auto entity = data_.entity[i];
  auto last_entity = data_.entity[last];

  swap(instance, last_instance);
  hash::set(map_, last_entity.id, i);
  hash::remove(map_, entity.id);

  --data_.size;
}

void TransformComponent::collect_garbage(const EntityManager &em) {
  const auto kAliveInARowThreshold = 4u;
  auto alive_in_row = 0u;
  while (data_.size > 0 && alive_in_row < kAliveInARowThreshold) {
    auto i = random_in_range(0u, data_.size - 1u);
    if (em.Alive(data_.entity[i])) {
      ++alive_in_row;
      continue;
    }
    alive_in_row = 0;
    destroy(i);
  }
}

bool TransformComponent::is_valid(Instance instance) const {
  return instance.i >= 0 && (uint32_t)instance.i < data_.size;
}

void TransformComponent::set_local(Instance instance, const glm::mat4 &local) {
  XASSERT(is_valid(instance), "Invalid instance");
  data_.local[instance.i] = local;
  auto parent = data_.parent[instance.i];
  auto parent_tm = is_valid(parent) ? data_.world[parent.i] : glm::mat4(1.0);
  transform(instance, parent_tm);
}

glm::mat4 TransformComponent::local(Instance instance) const {
  XASSERT(is_valid(instance), "Invalid instance");
  return data_.local[instance.i];
}

glm::mat4 TransformComponent::world(Instance instance) const {
  XASSERT(is_valid(instance), "Invalid instance");
  return data_.world[instance.i];
}

void TransformComponent::transform(Instance instance, const glm::mat4 &parent_world) {
  XASSERT(is_valid(instance), "Invalid instance");
  data_.world[instance.i] = data_.local[instance.i] * parent_world;

  auto child = data_.first_child[instance.i];
  while (is_valid(child)) {
    // TODO: Convert to iterative instead of recursive
    transform(child, data_.world[instance.i]); 
    child = data_.next_sibling[child.i];
  }
}

void TransformComponent::set_parent(Instance instance, Instance parent) {
  XASSERT(is_valid(instance), "Invalid child");

  if (data_.parent[instance.i].i != parent.i) {
    auto original_parent = data_.parent[instance.i];
    if (is_valid(original_parent)) {
      auto prev_sibling = data_.prev_sibling[instance.i];
      auto next_sibling = data_.next_sibling[instance.i];

      if (is_valid(next_sibling)) {
        data_.prev_sibling[next_sibling.i] = prev_sibling;
      }

      if (is_valid(prev_sibling)) {
        data_.next_sibling[prev_sibling.i] = next_sibling;
      } else {
        data_.first_child[original_parent.i] = next_sibling;
      }
    }

    data_.parent[instance.i] = parent;

    if (is_valid(parent)) {
      auto original_first_child = data_.first_child[parent.i];
      data_.first_child[parent.i] = instance;

      if (is_valid(original_first_child)) {
        data_.next_sibling[instance.i] = original_first_child;
        data_.prev_sibling[original_first_child.i] = instance;
      } else {
        data_.next_sibling[instance.i] = make_instance(-1);
      }

      data_.local[instance.i] = transform::get_relative(data_.world[parent.i], data_.world[instance.i]);
    }
  }
}

void TransformComponent::swap(Instance instance_a, Instance instance_b) {
  auto move_instance = [this](Instance instance, int index) {
    auto new_instance = make_instance(index);

    data_.entity[index] = data_.entity[instance.i];
    data_.local[index] = data_.local[instance.i];
    data_.world[index] = data_.world[instance.i];
    data_.parent[index] = data_.parent[instance.i];
    data_.first_child[index] = data_.first_child[instance.i];

    auto parent = data_.parent[index];

    if (is_valid(parent)) {
      auto next_sibling = data_.next_sibling[instance.i];
      data_.next_sibling[index] = next_sibling;
      if (is_valid(next_sibling)) {
        data_.prev_sibling[next_sibling.i] = new_instance;
      }

      auto prev_sibling = data_.prev_sibling[instance.i];
      data_.prev_sibling[index] = prev_sibling;
      if (is_valid(prev_sibling)) {
        data_.next_sibling[prev_sibling.i] = new_instance;
      } else {
        data_.first_child[parent.i] = new_instance;
      }
    }

    return new_instance;
  };

  auto a_index = instance_a.i;
  auto b_index = instance_b.i;

  instance_a = move_instance(instance_a, data_.size);
  move_instance(instance_b, a_index);
  move_instance(instance_a, b_index);
}

} // namespace knight
