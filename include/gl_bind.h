#ifndef KNIGHT_GL_BIND_H_
#define KNIGHT_GL_BIND_H_

#include "common.h"

#include <GL/glew.h>

namespace knight {

template<typename T>
class GlBind {
 public:
  explicit GlBind(const T &target) : target_(target) {
    target_.Bind();
  }

  ~GlBind() {
    target_.Unbind();
  }

 private:
  const T &target_;
};

} // namespace knight

#endif // KNIGHT_GL_BIND_H_