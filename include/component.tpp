// this file is included by component.h to implement template methods

namespace knight {

template<typename T>
std::shared_ptr<T> Component::Create() {
  return std::shared_ptr<T>(new T(Component::TypeFor<T>()));
}

// Generate and remember type for class T
template<typename T>
unsigned int Component::TypeFor() {
  static unsigned int type(Component::NextType());
  return type;
}

unsigned int Component::NextType() {
  static unsigned int type(0);
  return type++;
}

} // namespace knight
