#ifndef VOXY_MATH_MATRIX_TRANSFORM_H
#define VOXY_MATH_MATRIX_TRANSFORM_H

#include <voxy/math/vector.h>
#include <voxy/math/matrix.h>

static inline fmat4_t fmat4_translate(fvec3_t translation)
{
  fmat4_t mat = fmat4_identity();
  mat.values[0][3] = translation.x;
  mat.values[1][3] = translation.y;
  mat.values[2][3] = translation.z;
  return mat;
}

static inline fmat4_t fmat4_translate_inverse(fvec3_t translation)
{
  return fmat4_translate(fvec3_neg(translation));
}

static inline fmat4_t fmat4_rotate_x(float angle)
{
  fmat4_t result = fmat4_identity();
  result.values[1][1] =  cosf(angle);
  result.values[1][2] = -sinf(angle);
  result.values[2][1] =  sinf(angle);
  result.values[2][2] =  cosf(angle);
  return result;
}

static inline fmat4_t fmat4_rotate_y(float angle)
{
  fmat4_t result = fmat4_identity();
  result.values[2][2] =  cosf(angle);
  result.values[2][0] = -sinf(angle);
  result.values[0][2] =  sinf(angle);
  result.values[0][0] =  cosf(angle);
  return result;
}

static inline fmat4_t fmat4_rotate_z(float angle)
{
  fmat4_t result = fmat4_identity();
  result.values[0][0] =  cosf(angle);
  result.values[0][1] = -sinf(angle);
  result.values[1][0] =  sinf(angle);
  result.values[1][1] =  cosf(angle);
  return result;
}

static inline fmat4_t fmat4_rotate(fvec3_t rotation)
{
  fmat4_t mat = fmat4_identity();
  mat = fmat4_mul(fmat4_rotate_y(rotation.roll),  mat);
  mat = fmat4_mul(fmat4_rotate_x(rotation.pitch), mat);
  mat = fmat4_mul(fmat4_rotate_z(rotation.yaw),   mat);
  return mat;
}

static inline fmat4_t fmat4_rotate_inverse(fvec3_t rotation)
{
  fmat4_t mat = fmat4_identity();
  mat = fmat4_mul(fmat4_rotate_z(-rotation.yaw),   mat);
  mat = fmat4_mul(fmat4_rotate_x(-rotation.pitch), mat);
  mat = fmat4_mul(fmat4_rotate_y(-rotation.roll),  mat);
  return mat;
}

/*
 * This is to account for the difference in coordinate system between OpenGL and
 * us. In OpenGL x-axis points to the right, y-axis points upwards, and z-axis
 * points backward(out of screen). In our case, we have x-axis points to the
 * right, y-axis points forward(into screen) and z-axis points upwards.
 */
static inline fmat4_t fmat4_correction()
{
  fmat4_t result = fmat4_zero();
  result.values[0][0] =  1.0f;
  result.values[2][1] = -1.0f;
  result.values[1][2] =  1.0f;
  result.values[3][3] =  1.0f;
  return result;
}

static inline fmat4_t fmat4_perspecitve(float fovy, float aspect, float near, float far)
{
  fmat4_t result = fmat4_zero();
  result.values[0][0] = 1.0f / tanf(fovy/2.0f) / aspect;
  result.values[1][1] = 1.0f / tanf(fovy/2.0f);
  result.values[2][2] = -far              / (far - near);
  result.values[2][3] = -near*far / (far - near);
  result.values[3][2] = -1.0f;
  return result;
}

static inline fmat4_t fmat4_perspecitve_corrected(float fovy, float aspect, float near, float far)
{
  fmat4_t result = fmat4_identity();
  result = fmat4_mul(result, fmat4_correction());
  result = fmat4_mul(result, fmat4_perspecitve(fovy, aspect, near, far));
  return result;
}

/*
 * Apply a transformation matrix on a 3D vector without perspective divide.
 * Not correct if perspective matrix is used. Use
 * fmat4_apply_fvec3_perspective_divide instead.
 */
static inline fvec3_t fmat4_apply_fvec3(fmat4_t mat, fvec3_t vec)
{
  fvec4_t tmp;
  tmp = fvec4(vec.x, vec.y, vec.z, 1.0);
  tmp = fmat4_mul_vec(mat, tmp);
  return fvec3(tmp.x, tmp.y, tmp.z);
}

/*
 * Apply a transformation matrix on a 3D vector with perspective divide.
 */
static inline fvec3_t fmat4_apply_fvec3_perspective_divide(fmat4_t mat, fvec3_t vec)
{
  fvec4_t tmp;
  tmp = fvec4(vec.x, vec.y, vec.z, 1.0);
  tmp = fmat4_mul_vec(mat, tmp);
  return fvec3_div_scalar(fvec3(tmp.x, tmp.y, tmp.z), tmp.w);
}

#endif // VOXY_MATH_MATRIX_TRANSFORM_H
