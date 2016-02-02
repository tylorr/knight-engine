#include "transform_component.h"
#include "entity_manager.h"
#include "pointers.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <catch.hpp>

#include <ostream>

using namespace foundation;
using namespace knight;

namespace glm {
  std::ostream &operator<<(std::ostream &os, const mat4 &value) {
    os << to_string(value);
    return os;
  }
}

TEST_CASE("Transform Component") {
  auto &allocator = memory_globals::default_allocator();

  auto entity_manager = allocate_unique<EntityManager>(allocator, allocator);
  auto transform_component = allocate_unique<TransformComponent>(allocator, allocator);

  auto entity_id = entity_manager->create();
  auto entity = entity_manager->get(entity_id);

  transform_component->add(*entity);
  auto transform = transform_component->lookup(*entity);

  SECTION("Default matrixes are set to identity") {
    CHECK(transform_component->local(transform) == glm::mat4(1.0f));
    CHECK(transform_component->world(transform) == glm::mat4(1.0f));
  }

  auto new_transform_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, 0.0f));

  transform_component->set_local(transform, new_transform_matrix);

  CHECK(transform_component->local(transform) == new_transform_matrix);
  CHECK(transform_component->world(transform) == new_transform_matrix);

  SECTION("Growing allocation preserves data") {
    transform_component->allocate(transform_component->capacity() * 2);

    CHECK(transform_component->local(transform) == new_transform_matrix);
    CHECK(transform_component->world(transform) == new_transform_matrix);
  }

  SECTION("Parent child heirarchy") {
    auto parent_entity_id = entity_manager->create();
    auto parent_entity = entity_manager->get(parent_entity_id);

    transform_component->add(*parent_entity);
    auto parent_transform = transform_component->lookup(*parent_entity);

    transform_component->set_parent(transform, parent_transform);

    CHECK(transform_component->local(transform) == new_transform_matrix);
    CHECK(transform_component->world(transform) == new_transform_matrix);

    auto new_parent_transform_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.0f, 0.0f));

    transform_component->set_local(parent_transform, new_parent_transform_matrix);

    CHECK(transform_component->local(parent_transform) == new_parent_transform_matrix);
    CHECK(transform_component->world(parent_transform) == new_parent_transform_matrix);

    CHECK(transform_component->local(transform) == new_transform_matrix);
    CHECK(transform_component->world(transform) == glm::mat4(1.0f));
  }
}
