#ifndef MATERIAL_H_
#define MATERIAL_H_

#include "utils.h"

#include <glm/glm.hpp>

#include <GL/glew.h>
#include <GL/glfw.h>

#include <string>

class Material {
 public:
  explicit Material(const std::string &shader_key);
  ~Material() { }

  void UpdateUniforms() const;

  void PushMatrices(const glm::mat4 &model_view,
                    const glm::mat4 &projection,
                    const glm::mat4 &mvp,
                    const glm::mat3 &normal) const;
 private:
  GLuint shader_program_;

  GLint model_view_uniform_;
  GLint projection_uniform_;
  GLint mvp_uniform_;
  GLint normal_uniform_;

  DISALLOW_COPY_AND_ASSIGN(Material);
};

#endif // MATERIAL_H_
