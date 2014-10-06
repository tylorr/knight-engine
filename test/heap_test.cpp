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

    array::push_back(v, 13);
    array::push_back(v, 10);
    array::push_back(v, 7);
    array::push_back(v, 6);
    array::push_back(v, 9);
    array::push_back(v, 4);

    SECTION("Identity valid heap") {
      CHECK(heap::is_valid(array::begin(v), array::end(v)));
    }

    array::clear(v);

    const int max_value = 9;
    for (int i = 0; i <= max_value; ++i) {
      array::push_back(v, i);
    }

    SECTION("Identity invalid heap") {
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

    SECTION("Valid after push") {
      CHECK(heap::is_valid(array::begin(v), array::end(v)));
    }
  }
  memory_globals::shutdown();
}
