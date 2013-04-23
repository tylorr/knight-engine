#ifndef UTILS_H_
#define UTILS_H_

typedef struct Vertex
{
    float Position[4];
    float UV[2];
} Vertex;

void ExitOnGLError(const char *error_message);

#endif // UTILS_H_
