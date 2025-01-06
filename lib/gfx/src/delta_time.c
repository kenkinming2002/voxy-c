#include <libgfx/delta_time.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdbool.h>

float get_delta_time()
{
  static bool  initialized;
  static float prev_time;
  float        next_time;
  float        delta_time;

  if(!initialized)
  {
    initialized = true;
    prev_time = next_time = glfwGetTime();
    delta_time = 0.0f;
  }
  else
  {
    next_time = glfwGetTime();
    delta_time = next_time - prev_time;
    prev_time = next_time;
  }
  return delta_time;
}
