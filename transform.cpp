#include "transform.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/quaternion.hpp>

#include <iostream>

using glm::mat4;
using glm::mat3;
using glm::vec3;
using glm::quat;

using std::cout;
using std::endl;

Transform::Transform() {
  up_ = vec3(0, 1, 0);
}

Transform::~Transform() { }

void Transform::UpdateMatrix() {
  matrix_ = glm::translate(mat4(1.0f), position_);
  matrix_ = matrix_ * glm::toMat4(rotation_);
}

void Transform::TranslateOnAxis(const glm::vec3 &axis, const float &distance) {
  vec3 v1 = glm::rotate(rotation_, axis);

  position_ += (v1 * distance);
}

void Transform::TranslateX(const float &distance) {
  TranslateOnAxis(vec3(1, 0, 0), distance);
}

void Transform::TranslateY(const float &distance) {
  TranslateOnAxis(vec3(0, 1, 0), distance);
}

void Transform::TranslateZ(const float &distance) {
  TranslateOnAxis(vec3(0, 0, -1), distance);
}
