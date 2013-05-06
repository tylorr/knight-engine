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

  // Get the location of uniform from the shader associated with this material
  // and cache it. If the location is -1 that means the uniform does not exist
  const GLint &GetLocation(const std::string &name);

  // Checks to see if the the uniform with this name exists
  bool HasProperty(const std::string &name);

  // ---------------------------------------------------------------
  // Pass values to the shader at the specified name. If the name
  // does not exist on the shader then it is silently ignored.
  // ---------------------------------------------------------------

  void SetMatrix(const std::string &name, const glm::mat4 &matrix);
  void SetVector(const std::string &name, const glm::vec4 &vector);
  void SetFloat(const std::string &name, const float &value);

  // --------------------------------------------------------------
  // Getters and setters
  // --------------------------------------------------------------

  const GLuint &shader_program() const { return shader_program_; }

  void set_transparent(const bool &transparent) { transparent_ = transparent; }
  const bool &transparent() const { return transparent_; }

 private:
  // Shader program id
  GLuint shader_program_;

  // Uniform location cache
  UniformMap uniform_map_;

  bool transparent_;

  DISALLOW_COPY_AND_ASSIGN(Material);
};

#endif // MATERIAL_H_
