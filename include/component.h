#ifndef COMPONENT_H_
#define COMPONENT_H_

#include <memory>

namespace knight {

class Component {
 public:

  // Create pointer to component passing it a component Type
  template<typename T>
  static std::shared_ptr<T> Create();

  // Generate and remember type for class T
  template<typename T>
  static unsigned int TypeFor();

  unsigned int type() const { return type_; }
  unsigned int flag() const { return 1 << type_; }

 protected:
  explicit Component(unsigned int type) : type_(type) { }

 private:
  Component();

  // Return an incremented type for each call
  static unsigned int NextType();

  unsigned int type_;
};

} // namespace knight

#include "component.tpp"

#endif // COMPONENT_H_
