#pragma once

#include "std_allocator.h"

#include <unordered_map>
#include <functional>
#include <atomic>

namespace knight {

template<typename ValueType>
class TypeMap {
  using Container = std::unordered_map<
    int, 
    ValueType, 
    std::hash<int>, 
    std::equal_to<int>, 
    StdAllocator<std::pair<const int, ValueType>>>;
 public:
  using iterator = typename Container::iterator;
  using const_iterator = typename Container::const_iterator;

  explicit TypeMap(foundation::Allocator &allocator) :
    container_{allocator} {}

  auto begin() { return container_.begin(); }
  auto end() { return container_.end(); }
  auto begin() const { return container_.begin(); }
  auto end() const { return container_.end(); }
  auto cbegin() const { return container_.cbegin(); }
  auto cend() const { return container_.cend(); }

  template<typename Key>
  auto find() -> iterator { return container_.find(type_id<Key>()); }

  template<typename Key>
  auto find() const -> const_iterator const { return container_.find(type_id<Key>()); }

  template<typename Key>
  void put(ValueType &&value) {
    container_[type_id<Key>()] = std::forward<ValueType>(value);
  }

  template <class Key>
  static int type_id() {
    static int id = ++last_type_id_;
    return id;
  }

 private:
  static std::atomic<int> last_type_id_;
  Container container_;
};

template <class T>
std::atomic<int> TypeMap<T>::last_type_id_(0);

} // namespace knight
