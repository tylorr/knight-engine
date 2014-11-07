#include "entity_manager.h"
#include "slot_map.h"
#include "entity.h"
#include "component.h"
#include "common.h"

#include <catch.hpp>
#include <memory.h>

using namespace knight;
using namespace foundation;

struct TestComponent1 : public Component<TestComponent1> {
 public:
  TestComponent1() : x_(0) { }
  TestComponent1(int x) : x_(x) { }

  int x_;
};

struct TestComponent2 : public Component<TestComponent2> {
  TestComponent2() { }
};

TEST_CASE("Entity Manager Test") {
  Allocator &a = memory_globals::default_allocator();
  {
    EntityManager entity_manager_(a);
    Entity::ID id_ = entity_manager_.Create();

    SECTION("Add component in place") {
      int x = 3;
      auto component = entity_manager_.AddComponent<TestComponent1>(id_, x);
      REQUIRE(component != nullptr);

      CHECK(component->x_ == x);

      CHECK(entity_manager_.HasComponent<TestComponent1>(id_));
      CHECK(entity_manager_.component_mask(id_).test(TestComponent1::family()));
  }

  SECTION("Add component by value") {
    auto component = std::make_shared<TestComponent1>();
    entity_manager_.AddComponent(id_, component);

    CHECK(entity_manager_.HasComponent<TestComponent1>(id_));
    CHECK(entity_manager_.component_mask(id_).test(TestComponent1::family()));
  }

  SECTION("Getting non-existent component returns nullptr") {
    auto component2 = entity_manager_.GetComponent<TestComponent2>(id_);
    CHECK(component2 == nullptr);
  }

  auto component = entity_manager_.AddComponent<TestComponent1>(id_);

  SECTION("Get component by type returns correct component") {
    auto component2 = entity_manager_.GetComponent<TestComponent1>(id_);
    CHECK(component == component2);
  }

  SECTION("Removing component make component inaccessible") {
    entity_manager_.RemoveComponent<TestComponent1>(id_);

    auto component2 = entity_manager_.GetComponent<TestComponent1>(id_);
    CHECK(component2 == nullptr);
    CHECK(!entity_manager_.HasComponent<TestComponent1>(id_));
  }
}
}
