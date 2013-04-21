#include "script.h"
#include "utils.h"
#include "camera.h"

#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <tinythread.h>

#include <iostream>

using namespace tthread;
using glm::vec3;
using std::cout;
using std::endl;

#define WINDOW_TITLE_PREFIX "Chapter 1"

int CurrentWidth = 800,
    CurrentHeight = 600,
    WindowHandle = 0;

unsigned FrameCount = 0;

bool runFrameTimer = true;
bool TitleUpdated = false;

char* TitleString;

GLuint textureID;

GLuint
    ProjectionMatrixUniformLocation,
    ViewMatrixUniformLocation,
    ModelMatrixUniformLocation,
    BufferIds[3] = { 0 },
    ShaderIds[3] = { 0 };

glm::mat4
    ProjectionMatrix,
    ViewMatrix,
    ModelMatrix;

Camera *camera;

float CubeRotation;
double last_time = 0;
double elapsed_time = 0;

void Initialize();
void InitWindow();
void GLFWCALL ResizeFunction(int, int);
void RenderFunction(void);
void FramesTimer(void* arg);
void Cleanup(void);
void CreateShader(void);
void DestroyShader(void);
void CreateCube(void);
void DestroyCube(void);
void DrawCube(void);

int main(void)
{
    Initialize();

    last_time = glfwGetTime();

    int running = GL_TRUE;

    thread frameThread(FramesTimer, 0);

    // Main loop
    while (running)
    {
        double current_time = glfwGetTime();
        elapsed_time = current_time - last_time;
        last_time = current_time;
        if (TitleUpdated) {
            glfwSetWindowTitle(TitleString);
            TitleUpdated = false;
        }

        RenderFunction();

        // Check if ESC key was pressed or window was closed
        running = !glfwGetKey(GLFW_KEY_ESC) &&
                   glfwGetWindowParam(GLFW_OPENED);
    }

    runFrameTimer = false;
    frameThread.join();

    Cleanup();

    // Close window and terminate GLFW
    glfwTerminate();

    // Exit program
    exit(EXIT_SUCCESS);
}

void Initialize(void)
{
    GLenum GlewInitResult;

    InitWindow();

    glewExperimental = GL_TRUE;
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

    glGetError();
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    ExitOnGLError("ERROR: Could not set OpenGL depth testing options");

    // glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    ExitOnGLError("ERROR: Could not set OpenGL culling options");

    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    camera = new Camera();
    camera->position_ = vec3(0, 0, 2);
    // camera->rotation_ = glm::angleAxis(180.0f, vec3(0, 1, 0));
    // camera->rotation_ = glm::quat(vec3(0, 20, 0));

    ModelMatrix = glm::mat4(1.0f);
    ProjectionMatrix = glm::mat4(1.0f);

    // ViewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -2));


    CreateShader();
    CreateCube();

    int x, y, n;
    unsigned char *data = stbi_load("textures/CenterPiece.png", &x, &y, &n, 0);

    // printf("%i\n", x);
    // printf("%i\n", y);
    // printf("%i\n", n);

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    free(data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    GLuint color1Location = glGetUniformLocation(ShaderIds[0], "color1");
    GLuint color2Location = glGetUniformLocation(ShaderIds[0], "color2");
    GLuint color3Location = glGetUniformLocation(ShaderIds[0], "color3");
    GLuint color4Location = glGetUniformLocation(ShaderIds[0], "color4");

    glUseProgram(ShaderIds[0]);
    glUniform4f(color1Location, 0.0f, 0.0f, 0.0f, 0.0f);
    glUniform4f(color2Location, 0.0f, 1.0f, 0.0f, 0.33f);
    glUniform4f(color3Location, 0.0f, 0.0f, 1.0f, 0.66f);
    glUniform4f(color4Location, 1.0f, 1.0f, 0.0f, 1.0f);
    glUseProgram(0);

    glfwSetWindowSizeCallback(ResizeFunction);



    // Script script;

    // if (!script.loadScript("scripts/test.lua")) {
    //     fprintf(
    //         stderr,
    //         "Could not load script\n"
    //     );
    // }

    // std::string name = script.getGlobalString("PROGRAM_NAME");
    // printf("Program name: %s\n", name.c_str());
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

    // 1 for vsync on, 0 vsync off
    glfwSwapInterval(0);
}

void GLFWCALL ResizeFunction(int width, int height)
{
    CurrentWidth = width;
    CurrentHeight = height;

    glViewport(0, 0, CurrentWidth, CurrentHeight);

    ProjectionMatrix =
        glm::perspective(
            60.0f,
            (float)CurrentWidth / CurrentHeight,
            0.1f,
            100.0f
        );

    glUseProgram(ShaderIds[0]);
    glUniformMatrix4fv(ProjectionMatrixUniformLocation, 1, GL_FALSE, glm::value_ptr(ProjectionMatrix));
    glUseProgram(0);
}

int mouse_x;
int mouse_y;
int old_mouse_x;
int old_mouse_y;
float rot_speed = 20;
float translate_speed = 2;
vec3 rot(0);

void RenderFunction()
{
    ++FrameCount;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glfwGetMousePos(&mouse_x, &mouse_y);

    if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_RIGHT)) {
        int x_speed = (old_mouse_x - mouse_x) * rot_speed;
        int y_speed = (old_mouse_y - mouse_y) * rot_speed;

        rot.x += y_speed * (float)elapsed_time;
        rot.y += x_speed * (float)elapsed_time;
        // camera->rotation_ = glm::rotate(camera->rotation_, x_speed * (float)elapsed_time, vec3(0, 1, 0));
        camera->rotation_ = glm::quat(rot);

        // camera->rotation_.x -= y_diff * rot_speed * (float)elapsed_time;
        // camera->rotation_.y -= x_diff * rot_speed * (float)elapsed_time;

        if (glfwGetKey('W')) {
            // camera->position_ += camera->Forward() * translate_speed * (float)elapsed_time;
            camera->TranslateZ(translate_speed * (float)elapsed_time);
        }

        if (glfwGetKey('S')) {
            camera->TranslateZ(-translate_speed * (float)elapsed_time);
        }

        if (glfwGetKey('D')) {
            camera->TranslateX(translate_speed * (float)elapsed_time);
        }

        if (glfwGetKey('A')) {
            camera->TranslateX(-translate_speed * (float)elapsed_time);
        }

        if (glfwGetKey('E')) {
            camera->TranslateY(translate_speed * (float)elapsed_time);
        }

        if (glfwGetKey('Q')) {
            camera->TranslateY(-translate_speed * (float)elapsed_time);
        }
    }

    DrawCube();

    old_mouse_x = mouse_x;
    old_mouse_y = mouse_y;

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

void Cleanup(void)
{
    free(TitleString);
    delete camera;

    glDeleteTextures(1, &textureID);

    DestroyCube();
    DestroyShader();

}

void CreateShader(void)
{
    ShaderIds[0] = glCreateProgram();
    ExitOnGLError("ERROR: Could not create the shader program");
    {
        ShaderIds[1] = LoadShader("shaders/texture.frag", GL_FRAGMENT_SHADER);
        ShaderIds[2] = LoadShader("shaders/texture.vert", GL_VERTEX_SHADER);
        glAttachShader(ShaderIds[0], ShaderIds[1]);
        glAttachShader(ShaderIds[0], ShaderIds[2]);
    }
    glLinkProgram(ShaderIds[0]);
    ExitOnGLError("ERROR: Could not link the shader program");
}

void DestroyShader(void)
{
    glDetachShader(ShaderIds[0], ShaderIds[1]);
    glDetachShader(ShaderIds[0], ShaderIds[2]);
    glDeleteShader(ShaderIds[1]);
    glDeleteShader(ShaderIds[2]);
    glDeleteProgram(ShaderIds[0]);
    ExitOnGLError("ERROR: Could not destroy the shaders");
}

void CreateCube(void)
{
    const Vertex VERTICES[8] =
    {
        { { -.5f, -.5f,  .5f, 1 }, { 0, 1 } }, // 0 bottom left
        { { -.5f,  .5f,  .5f, 1 }, { 0, 0 } }, // 1 top left
        { {  .5f,  .5f,  .5f, 1 }, { 1, 0 } }, // 2 top right
        { {  .5f, -.5f,  .5f, 1 }, { 1, 1 } } // 3 bottom right
    };

    const GLuint INDICES[36] =
    {
        0,2,1,  0,3,2
    };

    ModelMatrixUniformLocation = glGetUniformLocation(ShaderIds[0], "ModelMatrix");
    ViewMatrixUniformLocation = glGetUniformLocation(ShaderIds[0], "ViewMatrix");
    ProjectionMatrixUniformLocation = glGetUniformLocation(ShaderIds[0], "ProjectionMatrix");
    ExitOnGLError("ERROR: Could not get shader uniform locations");

    glGenVertexArrays(1, &BufferIds[0]);
    ExitOnGLError("ERROR: Could not generate the VAO");
    glBindVertexArray(BufferIds[0]);
    ExitOnGLError("ERROR: Could not bind the VAO");

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    ExitOnGLError("ERROR: Could not enable vertex attributes");

    glGenBuffers(2, &BufferIds[1]);
    ExitOnGLError("ERROR: Could not generate the buffer objects");

    glBindBuffer(GL_ARRAY_BUFFER, BufferIds[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VERTICES), VERTICES, GL_STATIC_DRAW);
    ExitOnGLError("ERROR: Could not bind the VBO to the VAO");

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(VERTICES[0]), (GLvoid*)0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VERTICES[0]), (GLvoid*)sizeof(VERTICES[0].Position));
    ExitOnGLError("ERROR: Could not set VAO attributes");

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferIds[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(INDICES), INDICES, GL_STATIC_DRAW);
    ExitOnGLError("ERROR: Could not bind the IBO to the VAO");

    glBindVertexArray(0);
}

void DestroyCube()
{
    glDeleteBuffers(2, &BufferIds[1]);
    glDeleteVertexArrays(1, &BufferIds[0]);
    ExitOnGLError("ERROR: Could not destroy the buffer objects");
}

void DrawCube(void)
{

    ModelMatrix = glm::mat4(1.0f);
    // ModelMatrix = glm::rotate(
    //     glm::mat4(1.0f),
    //     CubeAngle, glm::vec3(1.0f, 0, 0));
    // ModelMatrix = glm::rotate(
    //     ModelMatrix,
    //     CubeAngle, glm::vec3(0, 1.0f, 0));

    glUseProgram(ShaderIds[0]);
    ExitOnGLError("ERROR: Could not use the shader program");

    glUniformMatrix4fv(ModelMatrixUniformLocation, 1, GL_FALSE, glm::value_ptr(ModelMatrix));


    ViewMatrix = camera->ViewMatrix();
    glUniformMatrix4fv(ViewMatrixUniformLocation, 1, GL_FALSE, glm::value_ptr(ViewMatrix));
    ExitOnGLError("ERROR: Could not set the shader uniforms");

    glBindVertexArray(BufferIds[0]);
    ExitOnGLError("ERROR: Could not bind the VAO for drawing purposes");

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (GLvoid*)0);
    ExitOnGLError("ERROR: Could not draw the cube");

    glBindVertexArray(0);
    glUseProgram(0);
}
