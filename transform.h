#ifndef TRANSFORM_H_
#define TRANSFORM_H_

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Transform {
public:
  glm::vec3 position_;
  glm::vec3 up_;
  glm::quat rotation_;

  glm::mat4 matrix_;

  Transform();
  virtual ~Transform();

  void TranslateOnAxis(const glm::vec3 &axis, const float &distance);
  void TranslateX(const float &distance);
  void TranslateY(const float &distance);
  void TranslateZ(const float &distance);

  void UpdateMatrix();
};

#endif // TRANSFORM_H_
