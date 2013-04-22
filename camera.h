#ifndef CAMERA_H_
#define CAMERA_H_

#include <glm/glm.hpp>

class Camera {
public:
  glm::mat4 projection_matrix_;

  Camera();
  virtual ~Camera();
};

#endif // CAMERA_H_
