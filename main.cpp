#include <GL/glew.h>
#include <GL/glfw.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define WINDOW_TITLE_PREFIX "Chapter 1"

int CurrentWidth = 800,
    CurrentHeight = 600,
    WindowHandle = 0;

unsigned FrameCount = 0;

void Initialize();
void InitWindow();
void GLFWCALL ResizeFunction(int, int);
void RenderFunction(void);
void TimerFunction(int);

int main(void)
{
    Initialize();

    int running = GL_TRUE;

    // Main loop
    while (running)
    {
        RenderFunction();

        // Check if ESC key was pressed or window was closed
        running = !glfwGetKey(GLFW_KEY_ESC) &&
                   glfwGetWindowParam(GLFW_OPENED);
    }

    // Close window and terminate GLFW
    glfwTerminate();

    // Exit program
    exit(EXIT_SUCCESS);
}

void Initialize(void)
{
    GLenum GlewInitResult;

    InitWindow();

    GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult) {
        fprintf(
            stderr,
            "ERROR: %s\n",
            glewGetErrorString(GlewInitResult)
        );
        exit(EXIT_FAILURE);
    }

    fprintf(
        stdout,
        "INFO: OpenGL Version: %s\n",
        glGetString(GL_VERSION)
    );

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
}

void InitWindow(void)
{
    if (!glfwInit())
    {
        exit(EXIT_FAILURE);
    }

    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 3);
    glfwOpenWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    if (!glfwOpenWindow(CurrentWidth, CurrentHeight, 0, 0, 0, 0, 24, 0, GLFW_WINDOW))
    {
        fprintf(
            stderr,
            "ERROR: Could not create a new rendering window.\n"
        );
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetWindowTitle(WINDOW_TITLE_PREFIX);

    glfwSetWindowSizeCallback(ResizeFunction);
}

void GLFWCALL ResizeFunction(int width, int height)
{
    CurrentWidth = width;
    CurrentHeight = height;

    glViewport(0, 0, CurrentWidth, CurrentHeight);
}

void RenderFunction()
{
    // OpenGL rendering goes here...
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Swap front and back rendering buffers
    glfwSwapBuffers();
}

