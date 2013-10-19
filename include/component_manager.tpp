// included by component_manager.h to implement template files

namespace knight {

template<typename T>
std::shared_ptr<T> ComponentManager::AddComponent(Entity &entity) {
  std::shared_ptr<T> component(Component::Create<T>());
  AddComponent(entity, component);
  return component;
}

template<typename T>
std::shared_ptr<T> ComponentManager::GetComponent(const Entity &entity) {
  return std::static_pointer_cast<T>(
    component_map_[Component::TypeFor<T>()][entity.id()]
  );
}

template<typename T>
void ComponentManager::RemoveComponent(Entity &entity) {
  auto component = component_map_[Component::TypeFor<T>()][entity.id()];
  RemoveComponent(entity, component);
}

}; // namespace knight
