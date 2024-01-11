#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <voxy/math.h>

struct mat4 mat4_translate(struct vec3 translation);

struct mat4 mat4_rotate_x(float angle);
struct mat4 mat4_rotate_y(float angle);
struct mat4 mat4_rotate_z(float angle);

struct mat4 mat4_rotate(struct vec3 rotation);

struct transform
{
  struct vec3 translation;
  struct vec3 rotation; // Euler angle
};

struct mat4 transform_matrix(struct transform *transform);
struct mat4 transform_matrix_inverse(struct transform *transform);

#endif // TRANSFORM_H
