#include <voxy/transform.h>

#include <math.h>

struct mat4 mat4_translate(struct vec3 translation)
{
  struct mat4 mat = mat4_identity();
  mat.values[0][3] = translation.x;
  mat.values[1][3] = translation.y;
  mat.values[2][3] = translation.z;
  return mat;
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

struct mat4 mat4_rotate(struct vec3 rotation)
{
  struct mat4 mat = mat4_identity();
  mat = mat4_mul(mat4_rotate_y(rotation.roll),  mat);
  mat = mat4_mul(mat4_rotate_x(rotation.pitch), mat);
  mat = mat4_mul(mat4_rotate_z(rotation.yaw),   mat);
  return mat;
}

struct mat4 transform_matrix(struct transform *transform)
{
  struct mat4 mat = mat4_identity();
  struct mat4 tmp;

  tmp = mat4_rotate(transform->rotation);
  mat = mat4_mul(tmp, mat);

  tmp = mat4_translate(transform->translation);
  mat = mat4_mul(tmp, mat);

  return mat;
}

struct mat4 transform_matrix_inverse(struct transform *transform)
{
  struct mat4 mat = mat4_identity();
  struct mat4 tmp;

  tmp = mat4_translate(vec3_neg(transform->translation));
  mat = mat4_mul(tmp, mat);

  tmp = mat4_rotate(vec3_neg(transform->rotation));
  mat = mat4_mul(tmp, mat);

  return mat;
}

