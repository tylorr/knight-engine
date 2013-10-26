#ifndef KNIGHT_COMPONENT_H_
#define KNIGHT_COMPONENT_H_

#include <cstdint>

namespace knight {

class ComponentBase {
 public:
  typedef uint64_t Family;
  // typedef uint64_t FamilyBit;

 protected:
  static Family next_family_;
};

template<typename T>
struct Component : public ComponentBase {
 public:
  static Family family();
  // static FamilyBit family_bit();
};

template<typename T>
ComponentBase::Family Component<T>::family() {
  static Family family(next_family_++);
  return family;
}

// template<typename T>
// ComponentBase::FamilyBit Component<T>::family_bit() {
//   return uint64_t(1) << family();
// }

} // namespace knight

#endif // KNIGHT_COMPONENT_H_
