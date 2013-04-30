#ifndef GEOMETRY_H_
#define GEOMETRY_H_

#include <glm/glm.hpp>

#include <vector>

struct Geometry {
 public:
  Geometry() { }
  ~Geometry() { }

  std::vector<glm::vec3> faces;
  std::vector<glm::vec3> vertices;
  std::vector<glm::vec3> normals;
  std::vector<glm::vec2> texture_coords;
};

#endif // GEOMETRY_H_
