#ifndef GEOMETRY_H_
#define GEOMETRY_H_

#include "utils.h"

#include <glm/glm.hpp>

#include <GL/glew.h>
#include <GL/glfw.h>

#include <vector>

class Geometry {
 public:
  Geometry() { }
  ~Geometry() { }

  void Bind(const bool &bind) const;

  void InitBuffers();
  void DestroyBuffers();

  void set_faces(const std::vector<glm::vec3> &faces) { faces_ = faces; }
  const std::vector<glm::vec3> &faces() const { return faces_; }

  void set_vertices(const std::vector<glm::vec3> &vertices) {
    vertices_ = vertices;
  }
  const std::vector<glm::vec3> &vertices() const { return vertices_; }

  void set_normals(const std::vector<glm::vec3> &normals) {
    normals_ = normals;
  }
  const std::vector<glm::vec3> &normals() const { return normals_; }

  void set_texture_coords(const std::vector<glm::vec2> &texture_coords) {
    texture_coords_ = texture_coords;
  }
  const std::vector<glm::vec2> &texture_coords() const {
    return texture_coords_;
  }

  unsigned int index_count() const {
    return faces_.size() * 3;
  }

 private:
  std::vector<glm::vec3> faces_;
  std::vector<glm::vec3> vertices_;
  std::vector<glm::vec3> normals_;
  std::vector<glm::vec2> texture_coords_;

  GLuint buffer_ids_[5];

  DISALLOW_COPY_AND_ASSIGN(Geometry);
};

#endif // GEOMETRY_H_
