#ifndef COMPONENT_H_
#define COMPONENT_H_

#include "common.h"

namespace knight {

class Component {
 public:
  Component(int type) : type_(type) { }

  template<typename T>
  static T *Create() {
    return new T(Component::TypeFor<T>());
  }

  int type() const { return type_; }

  ID id() const { return id_; }
  void set_id(ID id) { id_ = id; }

 private:
  static int NextType() {
    static int type(0);
    return type++;
  }

  template<typename T>
  static int TypeFor() {
    static int type(Component::NextType());
    return type;
  }

  int type_;
  ID id_;
};

}; // namespace knight

#endif // COMPONENT_H_
