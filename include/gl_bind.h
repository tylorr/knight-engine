#pragma once

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

  KNIGHT_DISALLOW_COPY_AND_ASSIGN(GlBind);
  KNIGHT_DISALLOW_MOVE_AND_ASSIGN(GlBind);
};

} // namespace knight
