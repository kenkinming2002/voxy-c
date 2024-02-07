#include "transform.h"

#include <math.h>

fmat4_t fmat4_translate(fvec3_t translation)
{
  fmat4_t mat = fmat4_identity();
  mat.values[0][3] = translation.x;
  mat.values[1][3] = translation.y;
  mat.values[2][3] = translation.z;
  return mat;
}

fmat4_t fmat4_translate_inverse(fvec3_t translation)
{
  return fmat4_translate(fvec3_neg(translation));
}

fmat4_t fmat4_rotate_x(float angle)
{
  fmat4_t result = fmat4_identity();
  result.values[1][1] =  cosf(angle);
  result.values[1][2] = -sinf(angle);
  result.values[2][1] =  sinf(angle);
  result.values[2][2] =  cosf(angle);
  return result;
}

fmat4_t fmat4_rotate_y(float angle)
{
  fmat4_t result = fmat4_identity();
  result.values[2][2] =  cosf(angle);
  result.values[2][0] = -sinf(angle);
  result.values[0][2] =  sinf(angle);
  result.values[0][0] =  cosf(angle);
  return result;
}

fmat4_t fmat4_rotate_z(float angle)
{
  fmat4_t result = fmat4_identity();
  result.values[0][0] =  cosf(angle);
  result.values[0][1] = -sinf(angle);
  result.values[1][0] =  sinf(angle);
  result.values[1][1] =  cosf(angle);
  return result;
}

fmat4_t fmat4_rotate(fvec3_t rotation)
{
  fmat4_t mat = fmat4_identity();
  mat = fmat4_mul(fmat4_rotate_y(rotation.roll),  mat);
  mat = fmat4_mul(fmat4_rotate_x(rotation.pitch), mat);
  mat = fmat4_mul(fmat4_rotate_z(rotation.yaw),   mat);
  return mat;
}

fmat4_t fmat4_rotate_inverse(fvec3_t rotation)
{
  fmat4_t mat = fmat4_identity();
  mat = fmat4_mul(fmat4_rotate_z(-rotation.yaw),   mat);
  mat = fmat4_mul(fmat4_rotate_x(-rotation.pitch), mat);
  mat = fmat4_mul(fmat4_rotate_y(-rotation.roll),  mat);
  return mat;
}

fmat4_t transform_matrix(struct transform *transform)
{
  fmat4_t mat = fmat4_identity();
  mat = fmat4_mul(fmat4_rotate(transform->rotation), mat);
  mat = fmat4_mul(fmat4_translate(transform->translation), mat);
  return mat;
}

fmat4_t transform_matrix_inverse(struct transform *transform)
{
  fmat4_t mat = fmat4_identity();
  mat = fmat4_mul(fmat4_translate_inverse(transform->translation), mat);
  mat = fmat4_mul(fmat4_rotate_inverse(transform->rotation), mat);
  return mat;
}

fvec3_t transform_local(struct transform *transform, fvec3_t translation)
{
  fvec4_t result = fmat4_mul_vec(fmat4_rotate(transform->rotation), fvec4(translation.x, translation.y, translation.z, 1.0f));
  return fvec3(result.x, result.y, result.z);
}

fvec3_t transform_right(const struct transform *transform)
{
  fvec4_t result = fmat4_mul_vec(fmat4_rotate(transform->rotation), fvec4(1.0f, 0.0f, 0.0f, 1.0f));
  return fvec3(result.x, result.y, result.z);
}

fvec3_t transform_forward(const struct transform *transform)
{
  fvec4_t result = fmat4_mul_vec(fmat4_rotate(transform->rotation), fvec4(0.0f, 1.0f, 0.0f, 1.0f));
  return fvec3(result.x, result.y, result.z);
}

fvec3_t transform_up(const struct transform *transform)
{
  fvec4_t result = fmat4_mul_vec(fmat4_rotate(transform->rotation), fvec4(0.0f, 0.0f, 1.0f, 1.0f));
  return fvec3(result.x, result.y, result.z);
}

void transform_rotate(struct transform *transform, fvec3_t rotation)
{
  transform->rotation = fvec3_add(transform->rotation, rotation);
}

void transform_local_translate(struct transform *transform, fvec3_t translation)
{
  fvec4_t offset = fmat4_mul_vec(fmat4_rotate(transform->rotation), fvec4(translation.x, translation.y, translation.z, 1.0f));
  transform->translation = fvec3_add(transform->translation, fvec3(offset.x, offset.y, offset.z));
}

