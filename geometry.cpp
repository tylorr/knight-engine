#include "geometry.h"

using glm::vec2;
using glm::vec3;
using std::vector;

void Geometry::Bind(const bool &bind) const {
  glBindVertexArray(bind ? buffer_ids_[0] : 0);
}

void Geometry::InitBuffers() {
  glGenVertexArrays(1, &buffer_ids_[0]);
  ExitOnGLError("ERROR: Could not generate the VAO");

  Bind(true);

  glGenBuffers(4, &buffer_ids_[1]);
  ExitOnGLError("ERROR: Could not generate the buffer objects");

  glBindBuffer(GL_ARRAY_BUFFER, buffer_ids_[1]);
  glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(vec3), &vertices_[0], GL_STATIC_DRAW);
  ExitOnGLError("ERROR: Could not bind vertices VBO");
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *)0);
  ExitOnGLError("ERROR: Could not set vertices attributes");

  glBindBuffer(GL_ARRAY_BUFFER, buffer_ids_[2]);
  glBufferData(GL_ARRAY_BUFFER, texture_coords_.size() * sizeof(vec2), &texture_coords_[0], GL_STATIC_DRAW);
  ExitOnGLError("ERROR: Could not bind text_coord VBO");
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *)0);
  ExitOnGLError("ERROR: Could not set text_coord attributes");

  glBindBuffer(GL_ARRAY_BUFFER, buffer_ids_[3]);
  glBufferData(GL_ARRAY_BUFFER, normals_.size() * sizeof(vec3), &normals_[0], GL_STATIC_DRAW);
  ExitOnGLError("ERROR: Could not bind normal VBO");
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *)0);
  ExitOnGLError("ERROR: Could not set normal attributes");

  // build index list
  vector<GLuint> indices;
  vector<vec3>::const_iterator it, end;
  for (it = faces_.begin(), end = faces_.end(); it != end; ++it) {
    indices.push_back((*it).x);
    indices.push_back((*it).y);
    indices.push_back((*it).z);
  }

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer_ids_[4]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);
  ExitOnGLError("ERROR: Could not bind the IBO to the VAO");

  Bind(false);
}

void Geometry::DestroyBuffers() {
  glDeleteBuffers(4, &buffer_ids_[1]);
  glDeleteVertexArrays(1, &buffer_ids_[0]);
}
