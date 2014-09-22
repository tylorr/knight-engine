#include "common.h"
#include "slot_map.h"

#include <catch.hpp>

using namespace knight;

struct Object {
  typedef ID32<Object>::ID ID;

  ID id_;
};

TEST_CASE("Slot Map") {
  SlotMap<Object, Object::ID> slot_map;

  Object::ID id;
  Object *object;

  SECTION("Invalid id will return a nullptr") {
    object = slot_map.Get(id);
    CHECK(object == nullptr);
  }

  id = slot_map.Create();

  SECTION("Valid id will return correct object") {
    object = slot_map.Get(id);

    REQUIRE(object != nullptr);
    CHECK(id == object->id_);
  }

  slot_map.Destroy(id);

  SECTION("Destroying object will invalidate id") {
    object = slot_map.Get(id);
    CHECK(object == nullptr);
  }

  SECTION("Objects created at same index will have different versions") {
    Object::ID new_id = slot_map.Create();

    CHECK(new_id != id);
    CHECK(new_id.index == id.index);
    CHECK(new_id.version == id.version + 1);
  }
}
