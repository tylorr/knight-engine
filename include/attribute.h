// citation: https://github.com/mosra/magnum/blob/03106cf5cbff3e9b17394a5f7d11bd3cba398e12/src/Magnum/Attribute.h

#pragma once

#include <GL/gl3w.h>

namespace knight {

namespace detail {
  template<typename> struct attribute_traits;
}

template<typename T>
class Attribute {
 public:
  using ScalarType = typename detail::attribute_traits<T>::ScalarType;
  using Components = typename detail::attribute_traits<T>::Components;
  using DataType = typename detail::attribute_traits<T>::DataType;
  using DataOption = typename detail::attribute_traits<T>::DataOption;

  Attribute(
    GLuint location,
    Components components, 
    DataType data_type = detail::attribute_traits<T>::default_data_type, 
    DataOption data_option = DataOption::None) :
      location_{location},
      components_{components},
      data_type_{data_type},
      data_option_{data_option} {}

  Attribute(
    GLuint location,
    DataType data_type = detail::attribute_traits<T>::default_data_type, 
    DataOption data_option = DataOption::None) :
      location_{location},
      components_{detail::attribute_traits<T>::default_components},
      data_type_{data_type},
      data_option_{data_option} {}

  auto components() const { return components_; }
  auto location() const { return location_; }
  auto data_type() const { return data_type_; }
  auto data_option() const { return data_option_; }


  GLint size() const {
    return detail::attribute_traits<T>::size(GLint(components_), data_type_);
  }

 private:
  GLuint location_;
  Components components_;
  DataType data_type_;
  DataOption data_option_;
};

namespace detail {

  template<GLint components>
  struct component_traits;

  template<>
  struct component_traits<1> {
    enum class Components : GLint { One = 1 };
    constexpr static Components default_components = Components::One;
  };

  template<>
  struct component_traits<2> {
    enum class Components : GLint { One = 1, Two = 2 };
    constexpr static Components default_components = Components::Two;
  };

  template<>
  struct component_traits<3> {
    enum class Components : GLint { One = 1, Two = 2, Three = 3 };
    constexpr static Components default_components = Components::Three;
  };

  template<>
  struct component_traits<4> {
    enum class Components : GLint { One = 1, Two = 2, Three = 3, Four = 4 };
    constexpr static Components default_components = Components::Four;
  };

  struct float_traits {
    using ScalarType = float;

    enum class DataType : GLenum {
      UnsignedByte = GL_UNSIGNED_BYTE,
      Byte = GL_BYTE,
      UnsignedShort = GL_UNSIGNED_SHORT,
      Short = GL_SHORT,
      UnsignedInt = GL_UNSIGNED_INT,
      Int = GL_INT,
      HalfFloat = GL_HALF_FLOAT,
      Float = GL_FLOAT,
      Double = GL_DOUBLE
    };
    constexpr static DataType default_data_type = DataType::Float;

    enum class DataOption {
      None,
      Normalized
    };

    static GLint size(GLint components, DataType dataType);
  };

  struct int_traits {
    using ScalarType = int;

    enum class DataType : GLenum {
      UnsignedByte = GL_UNSIGNED_BYTE,
      Byte = GL_BYTE,
      UnsignedShort = GL_UNSIGNED_SHORT,
      Short = GL_SHORT,
      UnsignedInt = GL_UNSIGNED_INT,
      Int = GL_INT
    };
    constexpr static DataType default_data_type = DataType::Int;

    enum class DataOption {
      None
    };

    static GLint size(GLint components, DataType dataType);
  };

  struct unsigned_int_traits {
    using ScalarType = unsigned int;

    using DataType = int_traits::DataType;
    constexpr static DataType default_data_type = DataType::UnsignedInt;

    using DataOption = int_traits::DataOption;

    static GLint size(GLint components, DataType dataType) {
      return int_traits::size(components, dataType);
    }
  };

  struct double_traits {
    using ScalarType = double;

    enum class DataType : GLenum {
        Double = GL_DOUBLE
    };
    constexpr static DataType default_data_type = DataType::Double;

    using DataOption = int_traits::DataOption;

    static GLint size(GLint components, DataType dataType);
  };

  template<>
  struct attribute_traits<float> : float_traits, component_traits<1> {};

  template<>
  struct attribute_traits<int> : int_traits, component_traits<1> {};

  template<>
  struct attribute_traits<unsigned int> : unsigned_int_traits, component_traits<1> {};

  template<>
  struct attribute_traits<double> : double_traits, component_traits<1> {};

} // namespace detail

} // namespace knight
