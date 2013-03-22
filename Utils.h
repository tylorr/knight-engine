#ifndef UTILS_H
#define UTILS_H

#define GLEW_STATIC

#include <GL/glew.h>
#include <GL/glfw.h>

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct Vertex
{
    float Position[4];
    float UV[2];
} Vertex;

void ExitOnGLError(const char* error_message);
GLuint LoadShader(const char* filename, GLenum shader_type);

#endif // UTILS_H
