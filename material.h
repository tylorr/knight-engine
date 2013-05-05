#ifndef MATERIAL_H_
#define MATERIAL_H_

#include "utils.h"

#include <glm/glm.hpp>

#include <GL/glew.h>
#include <GL/glfw.h>

#include <string>
#include <map>

class Material {
 public:
  typedef std::map<std::string, GLint> UniformMap;

  explicit Material(const std::string &shader_key);
  ~Material() { }

  const GLuint &shader_program() const { return shader_program_; }

  bool HasProperty(const std::string &name);

  const GLint &GetLocation(const std::string &name);

  void SetMatrix(const std::string &name, const glm::mat4 &matrix);
  void SetVector(const std::string &name, const glm::vec4 &vector);
  void SetFloat(const std::string &name, const float &value);

 private:
  GLuint shader_program_;

  UniformMap uniform_map_;

  DISALLOW_COPY_AND_ASSIGN(Material);
};

#endif // MATERIAL_H_
