#pragma once

#include <pcg_random.hpp>

#include <random>

namespace knight {

template<typename T>
std::enable_if_t<std::is_integral<T>::value, T> random_in_range(T a, T b);

template<typename T>
std::enable_if_t<std::is_floating_point<T>::value, T> random_in_range(T a, T b);

namespace random_internal {
  inline auto &prng_engine() {
    static std::atomic<uint64_t> thread_count;
    thread_local pcg_extras::seed_seq_from<std::random_device> seed_source;
    thread_local pcg32 engine{42, thread_count.fetch_add(1, std::memory_order_relaxed)};
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
