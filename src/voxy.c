#include <voxy/world.h>
#include <voxy/camera.h>
#include <voxy/renderer.h>
#include <voxy/shader.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define WINDOW_WIDTH  1024
#define WINDOW_HEIGHT 720
#define WINDOW_TITLE "voxy"

#define MOVE_SPEED 1.0f
#define PAN_SPEED  0.001f

static void glfw_error_callback(int error, const char *description)
{
  (void)error;
  fprintf(stderr, "ERROR: %s\n", description);
}

struct vertex
{
  struct vec3 position;
  struct vec3 color;
};

int main()
{
  glfwSetErrorCallback(&glfw_error_callback);
  if(!glfwInit())
  {
    fprintf(stderr, "ERROR: Failed to initialize GLFW\n");
    return EXIT_FAILURE;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

  GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
  if(!window)
  {
    fprintf(stderr, "ERROR: Failed to create window\n");
    glfwTerminate();
    return EXIT_FAILURE;
  }

  glfwMakeContextCurrent(window);
  if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    fprintf(stderr, "ERROR: Failed to load OpenGL\n");
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_FAILURE;
  }

  struct renderer renderer;
  struct world    world;
  struct camera   camera;

  world_init(&world);
  renderer_init(&renderer);

  struct chunk chunk;

  chunk.x = 0;
  chunk.y = 0;
  chunk.z = 0;
  chunk_randomize(&chunk);
  world_chunk_add(&world, chunk);

  chunk.x = 1;
  chunk.y = 0;
  chunk.z = 0;
  chunk_randomize(&chunk);
  world_chunk_add(&world, chunk);

  camera.transform.translation = vec3(10.0f, -10.0f, 0.0f);
  camera.transform.rotation    = vec3(0.0f, 0.0f, 0.0f);
  camera.fovy   = M_PI / 2.0f;
  camera.aspect = 1.0f;
  camera.near   = 0.1f;
  camera.far    = 50.0f;

  double xpos, ypos;
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwGetCursorPos(window, &xpos, &ypos);

  while(!glfwWindowShouldClose(window))
  {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderer_update(&renderer, &world);
    renderer_render(&renderer, &camera);

    glfwSwapBuffers(window);
    glfwPollEvents();

    camera.aspect = (float)width / (float)height;

    double new_xpos, new_ypos;
    glfwGetCursorPos(window, &new_xpos, &new_ypos);

    struct vec3 rotation = vec3_zero();

    rotation.yaw   = new_xpos - xpos;
    rotation.pitch = new_ypos - ypos;
    rotation = vec3_mul(rotation, PAN_SPEED);

    camera.transform.rotation = vec3_add(camera.transform.rotation, rotation);

    xpos = new_xpos;
    ypos = new_ypos;

    struct vec3 translation = vec3_zero();

    if(glfwGetKey(window, GLFW_KEY_D)) translation.x += 1.0f;
    if(glfwGetKey(window, GLFW_KEY_A)) translation.x -= 1.0f;

    if(glfwGetKey(window, GLFW_KEY_W)) translation.y += 1.0f;
    if(glfwGetKey(window, GLFW_KEY_S)) translation.y -= 1.0f;

    if(glfwGetKey(window, GLFW_KEY_SPACE))      translation.z += 1.0f;
    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) translation.z -= 1.0f;

    translation = vec3_normalize(translation);
    translation = vec3_mul(translation, MOVE_SPEED);

    transform_local_translate(&camera.transform, translation);
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  return EXIT_SUCCESS;
}
