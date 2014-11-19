#include <logog.hpp>
#include <memory.h>

#include <cstdio>

extern "C" {
  void Init();
  //void Update(double);
}

struct Foo { int x; };

void Init() {
  DBUG("Initialized script!!!");

  auto &allocator = foundation::memory_globals::default_allocator();

  auto foo = allocator.make_new<Foo>();

  INFO("%d", foo->x);

  allocator.make_delete(foo);
}

void Update(double dt) {
}
