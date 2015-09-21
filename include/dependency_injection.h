#pragma once

#include "common.h"
#include "pointers.h"
#include "type_map.h"
#include "pointer_wrapper.h"

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <stack>

namespace knight {

namespace di {

template<typename InstanceType, typename Deleter, class ...Deps>
using InstanceFactoryFunction = std::unique_ptr<InstanceType, Deleter>(*)(Deps *...);

class Injector {
  friend class InjectorConfig;
 public:
  Injector(Injector &&other) :
    instance_map_{std::move(other.instance_map_)} {}

  Injector &operator=(Injector &&other) { 
    instance_map_ = std::move(other.instance_map_);
    return *this;
  }

  template <typename T, typename Dependent = std::nullptr_t>
  auto get_instance() const -> T *;

  template<typename InstanceType, typename Deleter, typename ...Deps>
  auto Inject(
    InstanceFactoryFunction<InstanceType, Deleter, Deps...> 
      instance_factory) const -> std::unique_ptr<InstanceType, Deleter>;

 private:
  explicit Injector(foundation::Allocator &allocator) :
    instance_map_{allocator} {}

  using InstanceMap = TypeMap<pointer<PointerWrapper>>;

  InstanceMap instance_map_;
};

template<typename T, typename Dependent>
auto Injector::get_instance() const -> T * {
  auto it = instance_map_.find<T>();
  XASSERT(it != instance_map_.end(), "%s: unsatisfied dependency of %s", 
          typeid(T).name(), typeid(Dependent).name());

  return static_cast<T *>(it->second->get());
}

template<typename InstanceType, typename Deleter, typename ...Deps>
auto Injector::Inject(
  InstanceFactoryFunction<InstanceType, Deleter, Deps...> instance_factory) const 
    -> std::unique_ptr<InstanceType, Deleter> {
  return 
    instance_factory(
      get_instance<typename std::remove_const<Deps>::type,
                   typename std::remove_const<InstanceType>::type>()...);
}

class InjectorConfig {
 public:
  template<typename InstanceType, typename Deleter, typename ...Deps>
  void Add(
    InstanceFactoryFunction<InstanceType, Deleter, Deps...>
      instance_factory);

  Injector BuildInjector(foundation::Allocator &allocator);

 private:
  using InitializerFn = std::function<void(Injector &)>;

  struct NodeInfo {
    enum class Mark {
      Unmarked, Temp, Perm
    };

    Mark mark_;
    InitializerFn initializer_;
    bool has_initializer_ = false;
    std::unordered_set<int> dependents_;

    const char *debug_type_name_;
  };

  void ToposortVisitNode(int node_id, std::unordered_set<int> &unmarked_nodes,
                         std::stack<InitializerFn *> &output);

  std::unordered_map<int, NodeInfo> graph_;
};

template<typename ...Args>
inline void passthru(Args ...args) { }

template<typename InstanceType, typename Deleter, typename ...Deps>
void InjectorConfig::Add(
  InstanceFactoryFunction<InstanceType, Deleter, Deps...> 
    instance_factory) {

  int instance_type_id = TypeMap<NodeInfo>::type_id<typename std::remove_const<InstanceType>::type>();

  NodeInfo &new_node_info = graph_[instance_type_id];
  new_node_info.initializer_ = [instance_factory](Injector &inj) {
    auto instance = pointer_wrapper::create(inj.Inject(instance_factory));
    inj.instance_map_.put<InstanceType>(std::move(instance));
  };
  new_node_info.has_initializer_ = true;
  new_node_info.debug_type_name_ = typeid(typename std::remove_const<InstanceType>::type).name();

  passthru(
    graph_[TypeMap<NodeInfo>::type_id<typename std::remove_const<Deps>::type>()]
      .dependents_
      .insert(instance_type_id)...);  
}

} // namespace di

} // namespace knight
