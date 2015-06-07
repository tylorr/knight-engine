#pragma once

#include <random>

namespace knight {

template<typename T>
std::enable_if_t<std::is_integral<T>::value, T> random_in_range(T a, T b);

template<typename T>
std::enable_if_t<std::is_floating_point<T>::value, T> random_in_range(T a, T b);

namespace random_internal {
  inline auto &prng_engine() {
    thread_local static std::random_device rd{};
    thread_local static std::mt19937 engine{rd()};
    return engine;
  }
} // namespace random_internal


template<typename T>
std::enable_if_t<std::is_integral<T>::value, T> random_in_range(T a, T b) {
  std::uniform_int_distribution<T> dist{a, b};
  return dist(random_internal::prng_engine());
}

template<typename T>
std::enable_if_t<std::is_floating_point<T>::value, T> random_in_range(T a, T b) {
  std::uniform_real_distribution<T> dis{a, b};
  return dis(random_internal::prng_engine()); 
}

} // namespace knight
