#ifndef KNIGHT_COMMON_H_
#define KNIGHT_COMMON_H_

#include <cstdint>
#include <functional>

#define KNIGHT_DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName &) = delete;             \
  void operator=(const TypeName &) = delete;

namespace knight {

typedef unsigned int ComponentFlag;

union ID {
  uint64_t id;
  struct {
    uint32_t index;
    uint32_t version;
  };

  ID() : id(0) { }
  ID(const uint64_t &val) : id(val) { }
  ID(uint64_t &&val) : id(val) { }

  operator uint64_t() const { return id; }
  ID &operator=(const uint64_t &val) {
    id = val;
    return *this;
  }

  ID &operator=(uint64_t &&val) {
    id = val;
    return *this;
  }
};

} // namespace knight

namespace std {

// specialize std::hash for ID
template<>
struct hash<knight::ID> {
  size_t operator()(const knight::ID &id) const {
    return hash<uint64_t>()(id.id);
  }
};

}; // namespace std

#endif // KNIGHT_COMMON_H_
