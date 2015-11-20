#pragma once

#include "common.h"

namespace knight {

namespace templ {

template<typename... Args>
struct Reverse;

// recursion anchor
template<>
struct Reverse<> {
    template<typename Func, typename... Reversed>
    static void apply(const Func &func, Reversed&&... reversed) {
        EXPAND(func(std::forward<Reversed>(reversed)));
    }
};

// recursion
template<typename First, typename... Args>
struct Reverse<First, Args...>  {
    template<typename Func, typename... Reversed>
    static void apply(const Func &func, First&& first, Args&&... args, 
                      Reversed&&... reversed)
    {
        // bubble 1st parameter backwards
        Reverse<Args...>::apply(
          func,
          std::forward<Args>(args)..., 
          std::forward<First>(first), 
          std::forward<Reversed>(reversed)...);
    }
};

}

constexpr std::size_t sizeof_sum() { return 0; }

template<typename T, typename ...Args>
constexpr std::size_t sizeof_sum(const T &value, const Args&... args) {
  return sizeof(T) + sizeof_sum(args...);
}

} // namespace knight
