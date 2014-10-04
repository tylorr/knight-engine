#include "heap.h"
#include "array.h"
#include "memory.h"

#include <catch.hpp>

using namespace foundation;

TEST_CASE("Heapify an array") {
  memory_globals::init();
  Allocator &a = memory_globals::default_allocator();
  {
    Array<int> v(a);

    const int max_value = 9;
    for (int i = 0; i <= max_value; ++i) {
      array::push_back(v, i);
    }

    SECTION("Invalid before build") {
      CHECK(!heap::is_valid(array::begin(v), array::end(v)));
    }

    heap::build(array::begin(v), array::end(v));

    SECTION("Valid after build") {
      CHECK(heap::is_valid(array::begin(v), array::end(v)));
    }

    heap::pop(array::begin(v), array::end(v));

    SECTION("Pop returns max value") {
      CHECK(array::back(v) == max_value);
    }

    array::pop_back(v);

    SECTION("Valid after pop") {
      CHECK(heap::is_valid(array::begin(v), array::end(v)));
    }

    array::push_back(v, 10);
    heap::push(array::begin(v), array::end(v));

    // SECTION("Valid after push") {
    //   CHECK(heap::is_valid(array::begin(v), array::end(v)));
    // }

    // id = slot_map.Create();

    // SECTION("Valid id will return correct object") {
    //   object = slot_map.Get(id);

    //   REQUIRE(object != nullptr);
    //   CHECK(id == object->id_);
    // }

    // slot_map.Destroy(id);

    // SECTION("Destroying object will invalidate id") {
    //   object = slot_map.Get(id);
    //   CHECK(object == nullptr);
    // }

    // SECTION("Objects created at same index will have different versions") {
    //   Object::ID new_id = slot_map.Create();

    //   CHECK(new_id != id);
    //   CHECK(new_id.index == id.index);
    //   CHECK(new_id.version == id.version + 1);
    // }
  }
  memory_globals::shutdown();
}
