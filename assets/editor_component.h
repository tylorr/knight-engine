#pragma once

#include <gsl.h>

#include <unordered_map>

namespace knight {
namespace editor {

class Component {
 public:
  static std::unique_ptr<Component> create(const std::string &name);

  virtual std::unique_ptr<Component> clone() const = 0;

 protected:
  struct Prototype {};

  static void registerPrototype(std::string name, gsl::not_null<Component *> component);

 private:
  static std::unordered_map<std::string, Component *> _prototypes;
};

template <typename T>
class ComponentBase : public Component {
  using Component::Component;

  std::unique_ptr<Component> clone() const {
    return std::make_unique<T>(static_cast<const T &>(*this));
  }
};

} // namespace editor
} // namespace knight
