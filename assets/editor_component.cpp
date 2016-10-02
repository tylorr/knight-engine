#include "editor_component.h"

namespace knight {
namespace editor {

std::unordered_map<std::string, Component *> Component::_prototypes;

void Component::registerPrototype(std::string name, gsl::not_null<Component *> component) {
  _prototypes.emplace(std::move(name), component);
}

std::unique_ptr<Component> Component::create(const std::string &name) {
  auto prototypePair = _prototypes.find(name);
  if (prototypePair != _prototypes.end()) {
    return prototypePair->second->clone();
  }

  return nullptr;
}

} // namespace editor
} // namespace knight
