#ifndef VOXY_TRANSFORM_H
#define VOXY_TRANSFORM_H

#include "lin.h"

struct mat4 mat4_translate(struct vec3 translation);
struct mat4 mat4_translate_inverse(struct vec3 translation);

struct mat4 mat4_rotate_x(float angle);
struct mat4 mat4_rotate_y(float angle);
struct mat4 mat4_rotate_z(float angle);

struct mat4 mat4_rotate(struct vec3 rotation);
struct mat4 mat4_rotate_inverse(struct vec3 rotation);

struct transform
{
  struct vec3 translation;
  struct vec3 rotation; // Euler angle
};

struct mat4 transform_matrix(struct transform *transform);
struct mat4 transform_matrix_inverse(struct transform *transform);

struct vec3 transform_right  (struct transform *transform);
struct vec3 transform_forward(struct transform *transform);
struct vec3 transform_up     (struct transform *transform);

void transform_rotate(struct transform *transform, struct vec3 rotation);
void transform_local_translate(struct transform *transform, struct vec3 translation);

#endif // VOXY_TRANSFORM_H
