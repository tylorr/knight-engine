#pragma once

#include "common.h"
#include "std_allocator.h"

namespace knight {

class EntityManager;

template<typename Owner, typename T, size_t index_bits, size_t version_bits>
union ID {
  using type = T;

  T id;
  struct {
    T index   : index_bits;
    T version : version_bits;
  };

  ID() : id(0) { }
  ID(const T &val) : id(val) { }
  ID(T &&val) : id(val) { }

  operator T() const { return id; }
  ID &operator=(const T &val) {
    id = val;
    return *this;
  }

  bool operator==(const ID &other) const {
    return id == other.id;
  }
};

template<typename Owner>
struct ID32 { using ID = ID<Owner, uint32_t, 16, 16>; };
template<typename Owner>
struct ID64 { using ID = ID<Owner, uint64_t, 32, 32>; };

class Entity {
 public:
  using ID = ID64<Entity>::ID;
  ID id;

  Entity() : id(0) { }
};

template <typename T>
class Component;

class MeshComponent;

template<typename T, typename F>
class Copyer;

} // namespace knight
