#include "shader_cache.h"

#include "utils.h"

#include <map>
#include <cstdio>
#include <cassert>
#include <cstdlib>

using std::string;
using std::map;

namespace ShaderCache {

namespace {
  typedef map<string, ShaderInfo> ShaderMap;

  ShaderMap shader_map;
}; // namespace

// Load and compile a vertex or fragment shader, return the shader id
GLuint LoadShader(const std::string &filename, const GLenum &shader_type);

void AddShader(const std::string &key, const string &vertex_file, const string &fragment_file) {
  // Sanity check
  assert(vertex_file != fragment_file);

  GLuint program, vertex, fragment;
  ShaderMap::iterator it;

  it = shader_map.find(key);
  if (it != shader_map.end()) {
    fprintf(stderr, "ERROR: shader_cache: the key %s already exists\n", key.c_str());
    return;
  }

  program = glCreateProgram();
  ExitOnGLError("ERROR: Could not create the shader program");
  {
    fragment = LoadShader(fragment_file, GL_FRAGMENT_SHADER);
    vertex = LoadShader(vertex_file, GL_VERTEX_SHADER);
    glAttachShader(program, fragment);
    glAttachShader(program, vertex);
  }
  glLinkProgram(program);

  shader_map[key] = ShaderInfo(program, vertex, fragment);
}

GLuint GetShaderProgram(const std::string &key) {
  ShaderInfo si = shader_map.at(key);
  return si.program;
}

void Destroy() {
  GLuint program, vertex, fragment;
  ShaderMap::iterator it;

  for (it = shader_map.begin(); it != shader_map.end(); ++it) {
    program = it->second.program;
    vertex = it->second.vertex;
    fragment = it->second.fragment;

    glDetachShader(program, vertex);
    glDetachShader(program, fragment);
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    glDeleteProgram(program);
    ExitOnGLError("ERROR: Could not destroy the shaders");
  }

  shader_map.clear();
}

GLuint LoadShader(const string &filename, const GLenum &shader_type) {
  GLuint shader_id = 0;
  FILE* file;
  long file_size = -1;
  char* glsl_source;

  if (NULL != (file = fopen(filename.c_str(), "rb")) &&
    0 == fseek(file, 0, SEEK_END) &&
    -1 != (file_size = ftell(file)))
  {
    rewind(file);

    if (NULL != (glsl_source = (char*)malloc(file_size + 1)))
    {
      if (file_size == (long)fread(glsl_source, sizeof(char), file_size, file))
      {
        glsl_source[file_size] = '\0';

        if (0 != (shader_id = glCreateShader(shader_type)))
        {
          glShaderSource(shader_id, 1, (const GLchar **)&glsl_source, NULL);
          glCompileShader(shader_id);
          ExitOnGLError("Could not compile a shader");
        } else {
          fprintf(stderr, "ERROR: Could not create a shader.\n");
        }
      } else {
        fprintf(stderr, "ERROR: Could not read file %s\n", filename.c_str());
      }

      delete glsl_source;
    } else {
      fprintf(stderr, "ERROR: Could not allocate %li bytes.\n", file_size);
    }

    fclose(file);
  } else {
    fprintf(stderr, "ERROR: Could not open file %s\n", filename.c_str());
  }

  return shader_id;
}

}; // namespace ShaderCache
