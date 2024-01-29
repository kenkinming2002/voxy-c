#include "transform.h"

#include <math.h>

struct mat4 mat4_translate(fvec3_t translation)
{
  struct mat4 mat = mat4_identity();
  mat.values[0][3] = translation.x;
  mat.values[1][3] = translation.y;
  mat.values[2][3] = translation.z;
  return mat;
}

struct mat4 mat4_translate_inverse(fvec3_t translation)
{
  return mat4_translate(fvec3_neg(translation));
}

struct mat4 mat4_rotate_x(float angle)
{
  struct mat4 result = mat4_identity();
  result.values[1][1] =  cosf(angle);
  result.values[1][2] = -sinf(angle);
  result.values[2][1] =  sinf(angle);
  result.values[2][2] =  cosf(angle);
  return result;
}

struct mat4 mat4_rotate_y(float angle)
{
  struct mat4 result = mat4_identity();
  result.values[2][2] =  cosf(angle);
  result.values[2][0] = -sinf(angle);
  result.values[0][2] =  sinf(angle);
  result.values[0][0] =  cosf(angle);
  return result;
}

struct mat4 mat4_rotate_z(float angle)
{
  struct mat4 result = mat4_identity();
  result.values[0][0] =  cosf(angle);
  result.values[0][1] = -sinf(angle);
  result.values[1][0] =  sinf(angle);
  result.values[1][1] =  cosf(angle);
  return result;
}

struct mat4 mat4_rotate(fvec3_t rotation)
{
  struct mat4 mat = mat4_identity();
  mat = mat4_mul(mat4_rotate_y(rotation.roll),  mat);
  mat = mat4_mul(mat4_rotate_x(rotation.pitch), mat);
  mat = mat4_mul(mat4_rotate_z(rotation.yaw),   mat);
  return mat;
}

struct mat4 mat4_rotate_inverse(fvec3_t rotation)
{
  struct mat4 mat = mat4_identity();
  mat = mat4_mul(mat4_rotate_z(-rotation.yaw),   mat);
  mat = mat4_mul(mat4_rotate_x(-rotation.pitch), mat);
  mat = mat4_mul(mat4_rotate_y(-rotation.roll),  mat);
  return mat;
}

struct mat4 transform_matrix(struct transform *transform)
{
  struct mat4 mat = mat4_identity();
  mat = mat4_mul(mat4_rotate(transform->rotation), mat);
  mat = mat4_mul(mat4_translate(transform->translation), mat);
  return mat;
}

struct mat4 transform_matrix_inverse(struct transform *transform)
{
  struct mat4 mat = mat4_identity();
  mat = mat4_mul(mat4_translate_inverse(transform->translation), mat);
  mat = mat4_mul(mat4_rotate_inverse(transform->rotation), mat);
  return mat;
}

fvec3_t transform_right(struct transform *transform)
{
  fvec4_t result = mat4_mul_v(transform_matrix(transform), fvec4(1.0f, 0.0f, 0.0f, 1.0f));
  return fvec3(result.x, result.y, result.z);
}

fvec3_t transform_forward(struct transform *transform)
{
  fvec4_t result = mat4_mul_v(transform_matrix(transform), fvec4(0.0f, 1.0f, 0.0f, 1.0f));
  return fvec3(result.x, result.y, result.z);
}

fvec3_t transform_up(struct transform *transform)
{
  fvec4_t result = mat4_mul_v(transform_matrix(transform), fvec4(0.0f, 0.0f, 1.0f, 1.0f));
  return fvec3(result.x, result.y, result.z);
}

void transform_rotate(struct transform *transform, fvec3_t rotation)
{
  transform->rotation = fvec3_add(transform->rotation, rotation);
}

void transform_local_translate(struct transform *transform, fvec3_t translation)
{
  fvec4_t offset = mat4_mul_v(mat4_rotate(transform->rotation), fvec4(translation.x, translation.y, translation.z, 1.0f));
  transform->translation = fvec3_add(transform->translation, fvec3(offset.x, offset.y, offset.z));
}

