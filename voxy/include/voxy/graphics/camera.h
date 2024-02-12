#ifndef GRAPHICS_CAMERA_H
#define GRAPHICS_CAMERA_H

#include <voxy/math/transform.h>

struct camera
{
  transform_t transform;

  float fovy;
  float aspect;
  float near;
  float far;
};

fmat4_t camera_translation_matrix(struct camera *camera);
fmat4_t camera_rotation_matrix(struct camera *camera);
fmat4_t camera_view_matrix(struct camera *camera);
fmat4_t camera_projection_matrix(struct camera *camera);

#endif // GRAPHICS_CAMERA_H
