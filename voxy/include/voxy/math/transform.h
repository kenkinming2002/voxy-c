#ifndef VOXY_MATH_TRANSFORM_H
#define VOXY_MATH_TRANSFORM_H

#include <voxy/math/vector.h>
#include <voxy/math/matrix.h>
#include <voxy/math/matrix_transform.h>

typedef struct {
  fvec3_t translation;
  fvec3_t rotation; // Euler angle
} transform_t;

static inline fmat4_t transform_matrix(transform_t transform)
{
  fmat4_t mat = fmat4_identity();
  mat = fmat4_mul(fmat4_rotate(transform.rotation), mat);
  mat = fmat4_mul(fmat4_translate(transform.translation), mat);
  return mat;
}

static inline fmat4_t transform_matrix_inverse(transform_t transform)
{
  fmat4_t mat = fmat4_identity();
  mat = fmat4_mul(fmat4_translate_inverse(transform.translation), mat);
  mat = fmat4_mul(fmat4_rotate_inverse(transform.rotation), mat);
  return mat;
}

static inline fvec3_t transform_local(transform_t transform, fvec3_t translation)
{
  fvec4_t result = fmat4_mul_vec(fmat4_rotate(transform.rotation), fvec4(translation.x, translation.y, translation.z, 1.0f));
  return fvec3(result.x, result.y, result.z);
}

static inline fvec3_t transform_right(transform_t transform)
{
  fvec4_t result = fmat4_mul_vec(fmat4_rotate(transform.rotation), fvec4(1.0f, 0.0f, 0.0f, 1.0f));
  return fvec3(result.x, result.y, result.z);
}

static inline fvec3_t transform_forward(transform_t transform)
{
  fvec4_t result = fmat4_mul_vec(fmat4_rotate(transform.rotation), fvec4(0.0f, 1.0f, 0.0f, 1.0f));
  return fvec3(result.x, result.y, result.z);
}

static inline fvec3_t transform_up(transform_t transform)
{
  fvec4_t result = fmat4_mul_vec(fmat4_rotate(transform.rotation), fvec4(0.0f, 0.0f, 1.0f, 1.0f));
  return fvec3(result.x, result.y, result.z);
}

static inline transform_t transform_rotate(transform_t transform, fvec3_t rotation)
{
  transform.rotation = fvec3_add(transform.rotation, rotation);
  return transform;
}

static inline transform_t transform_local_translate(transform_t transform, fvec3_t translation)
{
  fvec4_t offset = fmat4_mul_vec(fmat4_rotate(transform.rotation), fvec4(translation.x, translation.y, translation.z, 1.0f));
  transform.translation = fvec3_add(transform.translation, fvec3(offset.x, offset.y, offset.z));
  return transform;
}

#endif // VOXY_MATH_TRANSFORM_H
