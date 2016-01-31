#include "dependency_injection.h"

namespace knight {

namespace di {

Injector InjectorConfig::build_injector(foundation::Allocator &allocator) {
  Injector inj{allocator};
  std::stack<InitializerFn *> initializers;

  std::unordered_set<int> unmarked_nodes;
  for (auto &&node : graph_) {
    node.second.mark_ = NodeInfo::Mark::Unmarked;
    unmarked_nodes.insert(node.first);
  }

  while (!unmarked_nodes.empty()) {
    int node_id = *unmarked_nodes.begin();
    toposort_visit_node(node_id, unmarked_nodes, initializers);
  }

  while (!initializers.empty()) {
    (*initializers.top())(inj);
    initializers.pop();
  }

  return inj;
}

void InjectorConfig::toposort_visit_node(
    int node_id, 
    std::unordered_set<int> &unmarked_nodes,
    std::stack<InitializerFn *> &output) {

  NodeInfo &info = graph_[node_id];
  XASSERT(info.mark_ != NodeInfo::Mark::Temp, "%s appears to be part of a cycle", info.debug_type_name_);

  if (info.mark_ == NodeInfo::Mark::Unmarked) {
    unmarked_nodes.erase(node_id);
    info.mark_ = NodeInfo::Mark::Temp;
    for (auto dependent : info.dependents_) {
      toposort_visit_node(dependent, unmarked_nodes, output);
    }
    info.mark_ = NodeInfo::Mark::Perm;
    if (info.has_initializer_) {
      output.push(&info.initializer_);
    }
  }
}


} // namespace di

} // namespace knight
