#include "attribute.h"

namespace knight {
namespace detail {
  GLint float_traits::size(GLint components, DataType dataType) {
    switch(dataType) {
      case DataType::UnsignedByte:
      case DataType::Byte:
        return sizeof(char) * components;
      case DataType::UnsignedShort:
      case DataType::Short:
      case DataType::HalfFloat:
        return sizeof(short) * components;
      case DataType::UnsignedInt:
      case DataType::Int:
      case DataType::Float:
        return sizeof(float) * components;
      case DataType::Double:
        return sizeof(double) * components;
      default: 
        return -1;
    }
  }

  GLint int_traits::size(GLint components, DataType dataType) {
    switch(dataType) {
      case DataType::UnsignedByte:
      case DataType::Byte:
        return sizeof(char) * components;
      case DataType::UnsignedShort:
      case DataType::Short:
        return sizeof(short) * components;
      case DataType::UnsignedInt:
      case DataType::Int:
        return sizeof(int) * components;
      default:
        return -1;
    }
  }

  GLint double_traits::size(GLint components, DataType dataType) {
    switch(dataType) {
      case DataType::Double:
        return sizeof(double) * components;
      default:
        return -1;
    }
  }

} // namespace detail
} // namespace knight
