#define LOGOG_LEVEL LOGOG_LEVEL_ALL

// #include <stb_image.h>
#include "shader.h"
#include "program.h"
#include "buffer_object.h"
#include "vertex_array.h"
#include "entity_manager.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

// #include <tinythread.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <logog.hpp>

#include <iostream>
#include <vector>
#include <functional>
#include <fstream>


// using namespace tthread;

using glm::vec4;
using glm::vec3;
using glm::vec2;
using std::cout;
using std::endl;
using std::vector;

using namespace knight;

#define WINDOW_TITLE_PREFIX "Chapter 1"

int CurrentWidth = 800,
  CurrentHeight = 600;

GLFWwindow *window;

unsigned FrameCount = 0;

bool runFrameTimer = true;
bool TitleUpdated = false;

char* TitleString;

GLuint textureID;

glm::mat4
  ProjectionMatrix,
  ViewMatrix,
  ModelMatrix;

// Transform *camera;
// GradientMapMaterial *material;
// Geometry *geometry;

float CubeRotation;
double last_time = 0;
double elapsed_time = 0;

bool Initialize();
bool InitWindow();
void ResizeFunction(int, int);
void RenderFunction(void);
void FramesTimer(void* arg);
void Cleanup(void);
void CreateShader(void);
void DestroyShader(void);
void CreateCube(void);
void DestroyCube(void);
void DrawCube(void);



int main(int argc, char *argv[]) {
  LOGOG_INITIALIZE();

  {
    // out must lose scope before LOGOG_SHUTDOWN()
    logog::Cout out;

    EntityManager entityManager;

    EntityID id = entityManager.CreateEntity();

    EntityID fake;
    // fake.id = 1;
    fake.index = 1;

    entityManager.DestroyEntity(fake);

    // id = entityManager.CreateEntity();



    // ID id = create_object();

    // object *obj = get_object(id);
    // obj->name = "theFirst";

    // id = create_object();

    // obj = get_object(id);
    // obj->name = "theSecond";

    // destroy_object(id);

    // id = create_object();

    // obj = get_object(id);
    // // obj->name = "theThird";

    printf("id:      %lu\n", id.id);
    printf("index:   %u\n", id.index);
    printf("version: %u\n", id.version);
    // printf("name:    %s\n", obj->name.c_str());

    // obj = get_object(id);

    // if (!obj) {
    //   printf("null\n");
    // }

    // long long id = 3;

    // union ID {
    //   long long id;
    //   struct {
    //     long index;
    //     long version;
    //   } s;
    // };

    // slot_map::ID id;

    // // id.index = 20;
    // // id.id = 30;
    // // id.version = 1;
    // id.version = 1;

    // printf("id: %lu\n", id);
    // printf("id: %ld\n", id.version);

    // printf("number: %lld\n", id);

    // if (Initialize()) {
    //   Shader vert(ShaderType::VERTEX, Shader::ReadSource("../shaders/simple.vert"));
    //   Shader frag(ShaderType::FRAGMENT, Shader::ReadSource("../shaders/simple.frag"));
    //   Program program(vert, frag);
    //   program.Bind();

    //   float vertices[] = {
    //     -0.5f,  0.5f, 0.0f,
    //      0.5f,  0.5f, 0.0f,
    //      0.5f, -0.5f, 0.0f
    //   };

    //   BufferObject vbo(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    //   VertexArray vao;
    //   vao.BindAttribute(vbo, program.GetAttribute("in_Position"),
    //                     3, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)0);
    //   // Main loop
    //   while (!glfwWindowShouldClose(window)) {

    //     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //     glDrawArrays(GL_TRIANGLES, 0, 3);
    //     // RenderFunction();

    //     glfwSwapBuffers(window);
    //     glfwPollEvents();
    //   }
    // }
  }

  // Close window and terminate GLFW
  glfwTerminate();

  LOGOG_SHUTDOWN();

  // Exit program
  exit(EXIT_SUCCESS);
}

bool Initialize()
{
  GLenum GlewInitResult;

  if (!InitWindow()) {
    return false;
  }

  glewExperimental = GL_TRUE;
  GlewInitResult = glewInit();

  if (GLEW_OK != GlewInitResult) {
    ERR("%s", glewGetErrorString(GlewInitResult));
    return false;
  }

  INFO("OpenGL Version: %s", glGetString(GL_VERSION));

  // glGetError();
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  // glEnable(GL_DEPTH_TEST);
  // glDepthFunc(GL_LESS);

  // // glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);

  // // glEnable(GL_BLEND);
  // // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // camera = new Transform();
  // camera->position_ = vec3(0, 0, 2);

  ModelMatrix = glm::mat4(1.0f);
  ProjectionMatrix = glm::mat4(1.0f);



  return true;

  // CreateShader();
  // CreateCube();

  // int x, y, n;
  // unsigned char *data = stbi_load("textures/CenterPiece.png", &x, &y, &n, 0);

  // // printf("%i\n", x);
  // // printf("%i\n", y);
  // // printf("%i\n", n);

  // glGenTextures(1, &textureID);
  // glBindTexture(GL_TEXTURE_2D, textureID);
  // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

  // free(data);

  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  // glGenerateMipmap(GL_TEXTURE_2D);

  // material = new GradientMapMaterial(ShaderCache::GetShaderProgram("gradient"));

  // material->set_color1(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  // material->set_color2(vec4(0.0f, 1.0f, 0.0f, 0.33f));
  // material->set_color3(vec4(0.0f, 0.0f, 1.0f, 0.66f));
  // material->set_color4(vec4(1.0f, 1.0f, 0.0f, 1.0f));

  // glfwSetWindowSizeCallback(ResizeFunction);


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

bool InitWindow(void) {
  if (!glfwInit()) {
    EMERGENCY("Unable to initialize GLFW library");
    return false;
  }

  window = glfwCreateWindow(CurrentWidth, CurrentHeight, "Hello World", NULL, NULL);

  if (!window) {
    EMERGENCY("Could not create a new rendering window");
    return false;
  }

  glfwMakeContextCurrent(window);

  return true;
}

/*
void GLFWCALL ResizeFunction(int width, int height)
{
  CurrentWidth = width;
  CurrentHeight = height;

  glViewport(0, 0, CurrentWidth, CurrentHeight);

  ProjectionMatrix = glm::perspective(
    60.0f,
    (float)CurrentWidth / CurrentHeight,
    0.1f,
    100.0f
  );

  // TODO: move this to render step
  material->Bind(true);
  material->SetMatrix("ProjectionMatrix", ProjectionMatrix);
  material->Bind(false);
}

int mouse_x;
int mouse_y;
int old_mouse_x;
int old_mouse_y;
float rot_speed = 10;
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
    camera->rotation_ = glm::quat(rot);

    float x_trans = 0;
    float y_trans = 0;
    float z_trans = 0;

    if (glfwGetKey('W')) {
      z_trans = translate_speed;
    }

    if (glfwGetKey('S')) {
      z_trans = -translate_speed;
    }

    if (glfwGetKey('D')) {
      x_trans = translate_speed;
    }

    if (glfwGetKey('A')) {
      x_trans = -translate_speed;
    }

    if (glfwGetKey('E')) {
      y_trans = translate_speed;
    }

    if (glfwGetKey('Q')) {
      y_trans = -translate_speed;
    }

    camera->TranslateX(x_trans * (float)elapsed_time);
    camera->TranslateY(y_trans * (float)elapsed_time);
    camera->TranslateZ(z_trans * (float)elapsed_time);
  }

  material->Bind(true);

  material->UpdateUniforms();


  camera->UpdateMatrix();
  ViewMatrix = glm::inverse(camera->matrix_);
  material->SetMatrix("ViewMatrix", ViewMatrix);


  DrawCube();

  material->Bind(false);

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
  delete TitleString;
  delete camera;

  glDeleteTextures(1, &textureID);

  DestroyCube();
  DestroyShader();

}

void CreateShader(void)
{
  ShaderCache::AddShader("gradient", "shaders/texture.vert", "shaders/texture.frag");
}

void DestroyShader(void)
{
  ShaderCache::Destroy();
}

void CreateCube() {
  geometry = new Geometry();

  vector<vec3> vertices;
  vertices.push_back(vec3(-.5f, -.5f,  .5f));
  vertices.push_back(vec3(-.5f,  .5f,  .5f));
  vertices.push_back(vec3( .5f,  .5f,  .5f));
  vertices.push_back(vec3( .5f, -.5f,  .5f));
  geometry->set_vertices(vertices);

  vector<vec3> normals;
  normals.push_back(vec3(0, 0, 1));
  normals.push_back(vec3(0, 0, 1));
  normals.push_back(vec3(0, 0, 1));
  normals.push_back(vec3(0, 0, 1));
  geometry->set_normals(normals);

  vector<vec2> texture_coords;
  texture_coords.push_back(vec2(0, 1));
  texture_coords.push_back(vec2(0, 0));
  texture_coords.push_back(vec2(1, 0));
  texture_coords.push_back(vec2(1, 1));
  geometry->set_texture_coords(texture_coords);

  vector<vec3> faces;
  faces.push_back(vec3(0, 2, 1));
  faces.push_back(vec3(0, 3, 2));
  geometry->set_faces(faces);

  geometry->InitBuffers();
}

void DestroyCube()
{
  geometry->DestroyBuffers();
}

void DrawCube(void)
{
  ModelMatrix = glm::mat4(1.0f);

  // glUniformMatrix4fv(ModelMatrixUniformLocation, 1, GL_FALSE, glm::value_ptr(ModelMatrix));
  material->SetMatrix("ModelMatrix", ModelMatrix);


  geometry->Bind(true);

  glDrawElements(GL_TRIANGLES, geometry->index_count(), GL_UNSIGNED_INT, (GLvoid*)0);

  geometry->Bind(false);
}
*/
