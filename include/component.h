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
  static int TypeFor() {
    static int type(Component::NextType());
    return type;
  }

  int type() const { return type_; }

 protected:
  explicit Component(int type) : type_(type) { }

 private:
  Component();

  // Return an incremented type for each call
  static int NextType() {
    static int type(0);
    return type++;
  }

  int type_;
};

}; // namespace knight

#endif // COMPONENT_H_
