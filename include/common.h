#ifndef COMMON_H_
#define COMMON_H_

#include <cstdint>

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

namespace knight {

union ID {
  uint64_t id;
  struct {
    uint32_t index;
    uint32_t version;
  };

  operator uint64_t() const { return id; }
  ID &operator=(const uint64_t &val) { id = val; return *this; }
};

}; // namespace knight

#endif // COMMON_H_
