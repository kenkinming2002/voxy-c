#include <graphics/camera.h>

#include <math.h>

fmat4_t camera_translation_matrix(struct camera *camera)
{
  return fmat4_translate_inverse(camera->transform.translation);
}

fmat4_t camera_rotation_matrix(struct camera *camera)
{
  return fmat4_rotate_inverse(camera->transform.rotation);
}

fmat4_t camera_view_matrix(struct camera *camera)
{
  // Who need matrix inverse?
  return transform_matrix_inverse(camera->transform);
}

fmat4_t camera_projection_matrix(struct camera *camera)
{
  fmat4_t result = fmat4_identity();
  fmat4_t tmp;

  // 1: Fixing axis orientation
  tmp = fmat4_zero();
  tmp.values[0][0] =  1.0f;
  tmp.values[2][1] = -1.0f;
  tmp.values[1][2] =  1.0f;
  tmp.values[3][3] =  1.0f;
  result = fmat4_mul(tmp, result);

  // 2: Actual perspective projection
  tmp = fmat4_zero();
  tmp.values[0][0] = 1.0f / tanf(camera->fovy/2.0f) / camera->aspect;
  tmp.values[1][1] = 1.0f / tanf(camera->fovy/2.0f);
  tmp.values[2][2] = -camera->far              / (camera->far - camera->near);
  tmp.values[2][3] = -camera->near*camera->far / (camera->far - camera->near);
  tmp.values[3][2] = -1.0f;
  result = fmat4_mul(tmp, result);

  return result;
}

