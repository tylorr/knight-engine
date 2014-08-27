#ifndef KNIGHT_COMMON_H_
#define KNIGHT_COMMON_H_

#include <bitset>
#include <cstdint>

#define KNIGHT_DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName &) = delete;            \
  void operator=(const TypeName &) = delete;

namespace knight {

// TODO: set from cmake
static const uint64_t MAX_COMPONENTS = 64;

typedef std::bitset<MAX_COMPONENTS> ComponentMask;

} // namespace knight

#endif // KNIGHT_COMMON_H_
