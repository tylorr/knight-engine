#ifndef KNIGHT_COMPONENT_H_
#define KNIGHT_COMPONENT_H_

#include "common.h"

#include <memory>

namespace knight {

class Component {
 public:

  // Create pointer to component passing it a component Type
  template<typename T>
  static std::shared_ptr<T> Create();

  // Generate and remember type for class T
  template<typename T>
  static ComponentFlag TypeFor();

  ComponentFlag type() const { return type_; }

 protected:
  explicit Component(ComponentFlag type) : type_(type) { }

 private:
  Component();

  // Return an incremented type for each call
  static ComponentFlag NextType() {
    static ComponentFlag bit(0);
    return 0x1 << bit++;
  }

  ComponentFlag type_;
};

template<typename T>
std::shared_ptr<T> Component::Create() {
  return std::shared_ptr<T>(new T(Component::TypeFor<T>()));
}

// Generate and remember type for class T
template<typename T>
ComponentFlag Component::TypeFor() {
  static ComponentFlag type(Component::NextType());
  return type;
}

} // namespace knight

#endif // KNIGHT_COMPONENT_H_
