#ifndef VOXY_CAMERA_H
#define VOXY_CAMERA_H

#include "transform.h"

struct camera
{
  struct transform transform;

  float fovy;
  float aspect;
  float near;
  float far;
};

fmat4_t camera_translation_matrix(struct camera *camera);
fmat4_t camera_rotation_matrix(struct camera *camera);
fmat4_t camera_view_matrix(struct camera *camera);
fmat4_t camera_projection_matrix(struct camera *camera);

#endif // VOXY_CAMERA_H
