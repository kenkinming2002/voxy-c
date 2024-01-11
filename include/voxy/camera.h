#ifndef CAMERA_H
#define CAMERA_H

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

#endif // CAMERA_H
