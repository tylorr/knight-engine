#ifndef UNIFORM_H_
#define UNIFORM_H_

#include <string>
#include <vector>

class Program;

enum UniformType {
  FLOAT_,
  FLOAT_VEC2,
  FLOAT_VEC3,
  FLOAT_VEC4,
  INT_
};

class Uniform {
 public:
  Uniform(Program *, const std::string &name, unsigned int location);
  Uniform(const std::string &name) : name_(name) { }

  void AddOwner(Program *, unsigned int);

  const std::string &name() { return name_; }

  const UniformType &type() { return type_; }

 protected:
  void NotifyOwners();

  UniformType type_;

 private:
  std::vector<Program *> owner_;
  std::vector<unsigned int> loc_;
  std::string name_;
};

template<typename T, unsigned int count, UniformType uniformType>
class UniformDerived : public Uniform {
 public:
  enum Attribs {
    kElementCount = count,
    kElementSize = sizeof(T)
  };

  UniformDerived(Program *sh, const std::string &name, int location)
      : Uniform(sh, name, location) {
    this->type_ = uniformType;        
  }

  UniformDerived(const std::string &name) 
      : Uniform(name) {
    this->type_ = uniformType;
  }

  void SetValue(T *values) {
    memcpy(elements_, values, kElementCount * kElementSize);
    NotifyOwners();
  }

  void Update() {
    UpdateImpl();
  }

  void UpdateImpl();

  T elements_[kElementCount];
};

typedef UniformDerived<float, 1, UniformType::FLOAT_> UniformFloat;
typedef UniformDerived<float, 2, UniformType::FLOAT_VEC2> UniformVec2;
typedef UniformDerived<float, 3, UniformType::FLOAT_VEC3> UniformVec3;
typedef UniformDerived<float, 4, UniformType::FLOAT_VEC4> UniformVec4;

typedef UniformDerived<int, 1, UniformType::INT_> UniformInt;

#endif // UNIFORM_H_
