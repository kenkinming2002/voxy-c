#include <libgfx/time.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

float get_time(void)
{
  return glfwGetTime();
}

