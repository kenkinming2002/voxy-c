#ifndef LIBCOMMON_GRAPHICS_CAMERA_H
#define LIBCOMMON_GRAPHICS_CAMERA_H

#include <libmath/transform.h>

struct camera
{
  transform_t transform;

  float fovy;
  float aspect;
  float near;
  float far;
};

fmat4_t camera_translation_matrix(const struct camera *camera);
fmat4_t camera_rotation_matrix(const struct camera *camera);
fmat4_t camera_view_matrix(const struct camera *camera);
fmat4_t camera_projection_matrix(const struct camera *camera);

#endif // LIBCOMMON_GRAPHICS_CAMERA_H
