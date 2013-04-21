#include <glm/glm.hpp>

class Camera {
public:
  glm::vec3 position_;
  glm::vec3 rotation_;

  Camera();
  virtual ~Camera();

  glm::mat4 ViewMatrix();

  glm::vec3 Forward();
  glm::vec3 Up();
  glm::vec3 Right();
private:
  glm::vec3 reference_;
  glm::vec3 world_up_;
};
