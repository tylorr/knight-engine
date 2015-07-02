#pragma once

namespace knight {

template<typename T, typename F>
class Copyer : public T {
 public:
  Copyer(T &&t) 
    : T(std::forward<T>(t)) { }

  Copyer(Copyer &&other) = default;
  Copyer &operator=(Copyer &&other) = default;

  Copyer(const Copyer &other) 
    : T(F()(other)) { }

  Copyer &operator=(const Copyer &other) {
      std::swap(*this, Copyer(other));
      return *this;
  }
};

}
