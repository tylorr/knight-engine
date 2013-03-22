#include "utils.h"

void ExitOnGLError(const char* error_message)
{
    const GLenum ErrorValue = glGetError();

    if (ErrorValue != GL_NO_ERROR)
    {
        const char* APPEND_DETAIL_STRING = ": %s\n";
        const size_t APPEND_LENGTH = strlen(APPEND_DETAIL_STRING) + 1;
        const size_t message_length = strlen(error_message);
        char* display_message = (char*)malloc(message_length + APPEND_LENGTH);

        memcpy(display_message, error_message, message_length);
        memcpy(&display_message[message_length], APPEND_DETAIL_STRING, APPEND_LENGTH);

        fprintf(stderr, display_message, gluErrorString(ErrorValue));

        free(display_message);
        exit(EXIT_FAILURE);
    }
}

GLuint LoadShader(const char* filename, GLenum shader_type)
{
    GLuint shader_id = 0;
    FILE* file;
    long file_size = -1;
    char* glsl_source;

    if (NULL != (file = fopen(filename, "rb")) &&
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
                    glShaderSource(shader_id, 1, (const GLchar**)&glsl_source, NULL);
                    glCompileShader(shader_id);
                    ExitOnGLError("Could not compile a shader");
                }
                else
                    fprintf(stderr, "ERROR: Could not create a shader.\n");
            }
            else
                fprintf(stderr, "ERROR: Could not read file %s\n", filename);

            free(glsl_source);
        }
        else
            fprintf(stderr, "ERROR: Could not allocate %lu bytes.\n", file_size);

        fclose(file);
    }
    else
        fprintf(stderr, "ERROR: Could not open file %s\n", filename);

    return shader_id;
}
