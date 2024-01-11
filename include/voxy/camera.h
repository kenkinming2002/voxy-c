#ifndef VOXY_CAMERA_H
#define VOXY_CAMERA_H

#include <voxy/transform.h>

struct camera
{
  struct transform transform;

  float fovy;
  float aspect;
  float near;
  float far;
};

struct mat4 camera_view_matrix(struct camera *camera);
struct mat4 camera_projection_matrix(struct camera *camera);

#endif // VOXY_CAMERA_H
