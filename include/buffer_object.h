#pragma once

#include "gsl.h"

#include <array.h>

#include <vector>

namespace knight {

class BufferObject {
 public:
  enum class Target : GLenum {
    Array = GL_ARRAY_BUFFER,
    AtomicCounter = GL_ATOMIC_COUNTER_BUFFER,
    CopyRead = GL_COPY_READ_BUFFER,
    CopyWrite = GL_COPY_WRITE_BUFFER,
    DispatchIndirect = GL_DISPATCH_INDIRECT_BUFFER,
    DrawIndirect = GL_DRAW_INDIRECT_BUFFER,
    ElementArray = GL_ELEMENT_ARRAY_BUFFER,
    PixelPack = GL_PIXEL_PACK_BUFFER,
    PixelUnpack = GL_PIXEL_UNPACK_BUFFER,
    ShaderStorage = GL_SHADER_STORAGE_BUFFER,
    Texture = GL_TEXTURE_BUFFER,
    TransformFeedback = GL_TRANSFORM_FEEDBACK_BUFFER,
    Uniform = GL_UNIFORM_BUFFER
  };

  enum class Usage : GLenum {
    StreamDraw = GL_STREAM_DRAW,
    StreamRead = GL_STREAM_READ,
    StreamCopy = GL_STREAM_COPY,

    StaticDraw = GL_STATIC_DRAW,
    StaticRead = GL_STATIC_READ,
    StaticCopy = GL_STATIC_COPY,

    DynamicDraw = GL_DYNAMIC_DRAW,
    DynamicRead = GL_DYNAMIC_READ,
    DynamicCopy = GL_DYNAMIC_COPY
  };

  BufferObject(Target target);

  BufferObject(const BufferObject &) = delete;
  BufferObject &operator=(const BufferObject &) = delete;

  BufferObject(BufferObject &&other);
  BufferObject &operator=(BufferObject &&other);

  ~BufferObject();

  GLuint handle() const { return handle_; }
  Target target() const { return target_; }

  void Bind() const;
  void Unbind() const;

  template<typename T>
  void SetData(gsl::span<T> data, Usage usage) {
    Bind();
    GL(glBufferData(GLenum(target_), data.size(), data.data(), GLenum(usage)));
  }

  void SetSubData(GLintptr offset, gsl::span<const char> data);

  template<typename T>
  void SetData(const foundation::Array<T> &data, BufferObject::Usage usage) {
    using namespace foundation;
    gsl::span<const T> span{array::begin(data), static_cast<long long>(array::size(data) * sizeof(T))};
    SetData(span, usage);
  }

  template<typename T>
  void SetData(const std::vector<T> &data, BufferObject::Usage usage) {
    using namespace foundation;
    gsl::span<const T> span{data.data(), static_cast<long long>(data.size() * sizeof(T))};
    SetData(span, usage);
  }

 private:
  GLuint handle_;
  Target target_;
};

} // namespace knight
