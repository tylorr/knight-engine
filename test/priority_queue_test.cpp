#include "priority_queue.h"
#include "array.h"
#include "memory.h"

#include <catch.hpp>

using namespace foundation;

TEST_CASE("Priority Queue") {
  Allocator &a = memory_globals::default_allocator();
  {
    Array<int> v(a);

    for (int i = 0; i < 3; ++i) {
      array::push_back(v, i);
    }

    PriorityQueue<int> pq(a, v);

    CHECK(priority_queue::size(pq) == 3);

    CHECK(priority_queue::top(pq) == 2);

    priority_queue::pop(pq);
    CHECK(priority_queue::size(pq) == 2);

    CHECK(priority_queue::top(pq) == 1);

    priority_queue::push(pq, 4);
    CHECK(priority_queue::size(pq) == 3);

    CHECK(priority_queue::top(pq) == 4);

    priority_queue::push(pq, 2);
    CHECK(priority_queue::size(pq) == 4);

    CHECK(priority_queue::top(pq) == 4);
  }
}
