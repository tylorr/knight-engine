#pragma once

#include <cstdint>

namespace knight {

class ComponentBase {
 public:
  typedef uint64_t Family;

 protected:
  static Family next_family_;
};

/// All components should inherit from Component in the form:
///
/// struct TestComponent : public Component<TestComponent> { }
///
/// All components must provide a default constructor.
template<typename T>
struct Component : public ComponentBase {
 public:
  static Family family();
};

template<typename T>
ComponentBase::Family Component<T>::family() {
  static Family family(next_family_++);
  return family;
}

} // namespace knight
