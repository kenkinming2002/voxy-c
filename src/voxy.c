#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <stdio.h>
#include <stdlib.h>

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 720
#define WINDOW_TITLE "voxy"

static void glfw_error_callback(int error, const char *description)
{
  fprintf(stderr, "ERROR: %s\n", description);
}

int main()
{
  glfwSetErrorCallback(&glfw_error_callback);
  if(!glfwInit())
  {
    fprintf(stderr, "ERROR: Failed to initialize GLFW\n");
    return EXIT_FAILURE;
  }

  GLFWwindow *window = NULL;

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  if(!(window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL)))
  {
    fprintf(stderr, "ERROR: Failed to create window\n");
    goto error;
  }

  glfwMakeContextCurrent(window);
  if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    fprintf(stderr, "ERROR: Failed to load OpenGL\n");
    goto error;
  }

  while(!glfwWindowShouldClose(window))
  {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

error:
  if(window)
    glfwDestroyWindow(window);

  glfwTerminate();
}
