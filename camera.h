#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Camera {
public:
  glm::vec3 position_;
  glm::quat rotation_;

  glm::mat4 matrix_;

  Camera();
  virtual ~Camera();

  glm::mat4 ViewMatrix();

  void TranslateOnAxis(const glm::vec3 &axis, const float &distance);
  void TranslateX(const float &distance);
  void TranslateY(const float &distance);
  void TranslateZ(const float &distance);

  void UpdateMatrix();
private:
  glm::vec3 reference_;
  glm::vec3 world_up_;
};
