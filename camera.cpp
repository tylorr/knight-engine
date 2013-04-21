#include "camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>

#include <iostream>

using glm::mat4;
using glm::mat3;
using glm::vec3;
using glm::quat;

using std::cout;
using std::endl;

Camera::Camera() {
  reference_ = glm::vec3(0, 0, -1);
  world_up_ = glm::vec3(0, 1, 0);
}

Camera::~Camera() { }

void Camera::UpdateMatrix()
{
  // matrix_ = glm::toMat4(rotation_);
  matrix_ = mat4(1.0f);
  matrix_ = glm::translate(matrix_, position_);
  matrix_ = matrix_ * glm::toMat4(rotation_);
  // matrix_ = glm::rotate(matrix_, rotation_);
}

mat4 Camera::ViewMatrix() {
  UpdateMatrix();
  return glm::inverse(matrix_);
}

void Camera::TranslateOnAxis(const glm::vec3 &axis, const float &distance)
{
  vec3 v1 = glm::rotate(rotation_, axis);

  position_ += (v1 * distance);
}

void Camera::TranslateX(const float &distance) {
  TranslateOnAxis(vec3(1, 0, 0), distance);
}

void Camera::TranslateY(const float &distance) {
  TranslateOnAxis(vec3(0, 1, 0), distance);
}

void Camera::TranslateZ(const float &distance) {
  TranslateOnAxis(vec3(0, 0, -1), distance);
}
