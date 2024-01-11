#include <voxy/camera.h>

#include <math.h>

struct mat4 camera_view_matrix(struct camera *camera)
{
  // Who need matrix inverse?
  return transform_matrix_inverse(&camera->transform);
}

struct mat4 camera_projection_matrix(struct camera *camera)
{
  struct mat4 result = mat4_identity();
  struct mat4 tmp;

  // 1: Fixing axis orientation
  tmp = mat4_zero();
  tmp.values[0][0] =  1.0f;
  tmp.values[2][1] = -1.0f;
  tmp.values[1][2] =  1.0f;
  tmp.values[3][3] =  1.0f;
  result = mat4_mul(tmp, result);

  // 2: Actual perspective projection
  tmp = mat4_zero();
  tmp.values[0][0] = 1.0f / tanf(camera->fovy/2.0f) / camera->aspect;
  tmp.values[1][1] = 1.0f / tanf(camera->fovy/2.0f);
  tmp.values[2][2] = -camera->far              / (camera->far - camera->near);
  tmp.values[2][3] = -camera->near*camera->far / (camera->far - camera->near);
  tmp.values[3][2] = -1.0f;
  result = mat4_mul(tmp, result);

  return result;
}

