#pragma once

#include "types.h"

#include <hash.h>
#include <memory_types.h>

namespace knight {

template<typename T>
class Component {
 public:
  struct Instance { int i; };

  Instance make_instance(int i);
  Instance lookup(Entity e);

 protected:
  Component(foundation::Allocator &alloc) : map_{alloc} { }

  foundation::Hash<uint32_t> map_;
};

template<typename T>
auto Component<T>::make_instance(int i) -> Instance {
  return Instance{i};
}

template<typename T>
auto Component<T>::lookup(Entity e) -> Instance {
  return make_instance(foundation::hash::get(map_, e.id, 0u));
}

} // namespace knight
