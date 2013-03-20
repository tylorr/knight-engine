#include <GL/glew.h>
#include <GL/glfw.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tinythread.h>

using namespace tthread;

#define WINDOW_TITLE_PREFIX "Chapter 1"

int CurrentWidth = 800,
    CurrentHeight = 600,
    WindowHandle = 0;

unsigned FrameCount = 0;

bool runFrameTimer = true;
bool TitleUpdated = false;

char* TitleString;

void Initialize();
void InitWindow();
void GLFWCALL ResizeFunction(int, int);
void RenderFunction(void);
void FramesTimer(void * arg);

int main(void)
{
    Initialize();

    int running = GL_TRUE;

    thread frameThread(FramesTimer, 0);

    // Main loop
    while (running)
    {
        if (TitleUpdated) {
            glfwSetWindowTitle(TitleString);
            TitleUpdated = false;
        }

        RenderFunction();


        // UpdateTitle();

        // Check if ESC key was pressed or window was closed
        running = !glfwGetKey(GLFW_KEY_ESC) &&
                   glfwGetWindowParam(GLFW_OPENED);
    }

    runFrameTimer = false;
    frameThread.join();

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
    ++FrameCount;
    // OpenGL rendering goes here...
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Swap front and back rendering buffers
    glfwSwapBuffers();
}

void FramesTimer(void * arg)
{
    while(runFrameTimer) {
        glfwSleep(0.25);

        TitleString = (char*)
            malloc(512 + strlen(WINDOW_TITLE_PREFIX));

        sprintf(
            TitleString,
            "%s: %d Frames Per Second @ %d x %d",
            WINDOW_TITLE_PREFIX,
            FrameCount * 4,
            CurrentWidth,
            CurrentHeight
        );

        TitleUpdated = true;
        FrameCount = 0;
    }
}

