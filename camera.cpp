#include "camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/string_cast.hpp>

#include <iostream>

using glm::mat4;
using glm::mat3;
using glm::vec3;

using std::cout;
using std::endl;

Camera::Camera() {
  reference_ = glm::vec3(0, 0, -1);
  world_up_ = glm::vec3(0, 1, 0);

  cout << glm::to_string(Up()) << endl;
}

Camera::~Camera() { }

vec3 Camera::Forward() {
  mat3 rot_mat = glm::orientate3(rotation_);
  vec3 forward = rot_mat * reference_;
  return forward;
}

vec3 Camera::Right() {
  return glm::cross(Forward(), world_up_);
}

vec3 Camera::Up() {
  return glm::cross(Right(), Forward());
}

mat4 Camera::ViewMatrix() {
  // mat3 rot_mat = glm::orientate3(rotation_);
  // vec3 trans_ref = rot_mat * reference_;
  vec3 center = position_ + Forward();
  return glm::lookAt(position_, center, world_up_);
}
