#ifndef COMPONENT_H_
#define COMPONENT_H_

#include <memory>

namespace knight {

class Component {
 public:

  // Create pointer to component passing it a component Type
  template<typename T>
  static std::shared_ptr<T> Create() {
    return std::shared_ptr<T>(new T(Component::TypeFor<T>()));
  }

  // Generate and remember type for class T
  template<typename T>
  static unsigned int TypeFor() {
    static unsigned int type(Component::NextType());
    return type;
  }

  unsigned int type() const { return type_; }

 protected:
  explicit Component(unsigned int type) : type_(type) { }

 private:
  Component();

  // Return an incremented type for each call
  static unsigned int NextType() {
    static unsigned int type(0);
    return type++;
  }

  unsigned int type_;
};

}; // namespace knight

#endif // COMPONENT_H_
