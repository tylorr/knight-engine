#include "material.h"

#include "utils.h"

#include <glm/glm.hpp>

class GradientMapMaterial : public Material {
 public:
  GradientMapMaterial(const GLuint &shader_program)
    : Material(shader_program) { }

  virtual void UpdateUniforms();

  void set_color1(const glm::vec4 &color1) { color1_ = color1; }
  const glm::vec4 &color1() const { return color1_; }

  void set_color2(const glm::vec4 &color2) { color2_ = color2; }
  const glm::vec4 &color2() const { return color2_; }

  void set_color3(const glm::vec4 &color3) { color3_ = color3; }
  const glm::vec4 &color3() const { return color3_; }

  void set_color4(const glm::vec4 &color4) { color4_ = color4; }
  const glm::vec4 &color4() const { return color4_; }

 private:
  glm::vec4 color1_;
  glm::vec4 color2_;
  glm::vec4 color3_;
  glm::vec4 color4_;

  DISALLOW_COPY_AND_ASSIGN(GradientMapMaterial);
};
