#ifndef SHADER_CAHCE_H_
#define SHADER_CAHCE_H_

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <string>

/*
 * Use these methods to load shaders into a cache of shader programs
 */
namespace ShaderCache {

// stores information about the shader program and shaders
struct ShaderInfo {
  GLuint program;
  GLuint vertex;
  GLuint fragment;

  ShaderInfo() {
    program = 0;
    vertex = 0;
    fragment = 0;
  }

  ShaderInfo(const GLuint &program, const GLuint &vertex, const GLuint &fragment)
    : program(program),
      vertex(vertex),
      fragment(fragment) { }
};

// Compile and link a shader program and store it with the given key
void AddShader(const std::string &key, const std::string &vertex_file, const std::string &fragment_file);

// Get shader program store with specified key
const GLuint &GetShaderProgram(const std::string &key);

// Delete all of the compiled shaders and clear the cache
void Destroy();

}; // namespace ShaderCache

#endif // SHADER_CAHCE_H_
