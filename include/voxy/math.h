#ifndef VOXY_MATH_H
#define VOXY_MATH_H

#include <math.h>

struct vec2
{
  union
  {
    float values[2];
    struct { float x, y; };
  };
};

struct vec3
{
  union
  {
    float values[3];
    struct { float x, y, z; };
    struct { float r, g, b; };
    struct { float yaw, pitch, roll; };
  };
};

struct vec4
{
  union
  {
    float values[4];
    struct { float x, y, z, w; };
    struct { float r, g, b, a; };
  };
};

struct mat2 { float values[2][2]; };
struct mat3 { float values[3][3]; };
struct mat4 { float values[4][4]; };

static inline struct vec2 vec2(float a, float b)                   { return (struct vec2){ .values = {a, b}       }; }
static inline struct vec3 vec3(float a, float b, float c)          { return (struct vec3){ .values = {a, b, c}    }; }
static inline struct vec4 vec4(float a, float b, float c, float d) { return (struct vec4){ .values = {a, b, c, d} }; }

static inline struct vec2 vec2_zero() { return vec2(0.0f, 0.0f); }
static inline struct vec3 vec3_zero() { return vec3(0.0f, 0.0f, 0.0f);       }
static inline struct vec4 vec4_zero() { return vec4(0.0f, 0.0f, 0.0f, 0.0f); }

static inline struct vec2 vec2_neg(struct vec2 vec)
{
  for(unsigned i=0; i<2; ++i)
    vec.values[i] = -vec.values[i];
  return vec;
}

static inline struct vec3 vec3_neg(struct vec3 vec)
{
  for(unsigned i=0; i<3; ++i)
    vec.values[i] = -vec.values[i];
  return vec;
}

static inline struct vec4 vec4_neg(struct vec4 vec)
{
  for(unsigned i=0; i<4; ++i)
    vec.values[i] = -vec.values[i];
  return vec;
}

static inline struct vec2 vec2_mul(struct vec2 vec, float value)
{
  for(unsigned i=0; i<2; ++i)
    vec.values[i] *= value;
  return vec;
}

static inline struct vec3 vec3_mul(struct vec3 vec, float value)
{
  for(unsigned i=0; i<3; ++i)
    vec.values[i] *= value;
  return vec;
}

static inline struct vec4 vec4_mul(struct vec4 vec, float value)
{
  for(unsigned i=0; i<4; ++i)
    vec.values[i] *= value;
  return vec;
}

static inline struct vec2 vec2_div(struct vec2 vec, float value)
{
  for(unsigned i=0; i<2; ++i)
    vec.values[i] /= value;
  return vec;
}

static inline struct vec3 vec3_div(struct vec3 vec, float value)
{
  for(unsigned i=0; i<3; ++i)
    vec.values[i] /= value;
  return vec;
}

static inline struct vec4 vec4_div(struct vec4 vec, float value)
{
  for(unsigned i=0; i<4; ++i)
    vec.values[i] /= value;
  return vec;
}

static inline struct vec2 vec2_add(struct vec2 lhs, struct vec2 rhs)
{
  struct vec2 result;
  for(unsigned i=0; i<2; ++i)
    result.values[i] = lhs.values[i] + rhs.values[i];
  return result;
}

static inline struct vec3 vec3_add(struct vec3 lhs, struct vec3 rhs)
{
  struct vec3 result;
  for(unsigned i=0; i<3; ++i)
    result.values[i] = lhs.values[i] + rhs.values[i];
  return result;
}

static inline struct vec4 vec4_add(struct vec4 lhs, struct vec4 rhs)
{
  struct vec4 result;
  for(unsigned i=0; i<4; ++i)
    result.values[i] = lhs.values[i] + rhs.values[i];
  return result;
}

static inline struct vec2 vec2_sub(struct vec2 lhs, struct vec2 rhs)
{
  struct vec2 result;
  for(unsigned i=0; i<2; ++i)
    result.values[i] = lhs.values[i] - rhs.values[i];
  return result;
}

static inline struct vec3 vec3_sub(struct vec3 lhs, struct vec3 rhs)
{
  struct vec3 result;
  for(unsigned i=0; i<3; ++i)
    result.values[i] = lhs.values[i] - rhs.values[i];
  return result;
}

static inline struct vec4 vec4_sub(struct vec4 lhs, struct vec4 rhs)
{
  struct vec4 result;
  for(unsigned i=0; i<4; ++i)
    result.values[i] = lhs.values[i] - rhs.values[i];
  return result;
}

static inline float vec2_dot(struct vec2 lhs, struct vec2 rhs)
{
  float result = 0.0f;
  for(unsigned i=0; i<2; ++i)
    result += lhs.values[i] * rhs.values[i];
  return result;
}

static inline float vec3_dot(struct vec3 lhs, struct vec3 rhs)
{
  float result = 0.0f;
  for(unsigned i=0; i<3; ++i)
    result += lhs.values[i] * rhs.values[i];
  return result;
}

static inline float vec4_dot(struct vec4 lhs, struct vec4 rhs)
{
  float result = 0.0f;
  for(unsigned i=0; i<4; ++i)
    result += lhs.values[i] * rhs.values[i];
  return result;
}

static inline float vec2_length_squared(struct vec2 vec) { return vec2_dot(vec, vec); }
static inline float vec3_length_squared(struct vec3 vec) { return vec3_dot(vec, vec); }
static inline float vec4_length_squared(struct vec4 vec) { return vec4_dot(vec, vec); }

static inline float vec2_length(struct vec2 vec) { return sqrtf(vec2_length_squared(vec)); }
static inline float vec3_length(struct vec3 vec) { return sqrtf(vec3_length_squared(vec)); }
static inline float vec4_length(struct vec4 vec) { return sqrtf(vec4_length_squared(vec)); }

static inline struct vec2 vec2_normalize(struct vec2 vec) { float length = vec2_length_squared(vec); return length != 0.0f ? vec2_div(vec, length) : vec; }
static inline struct vec3 vec3_normalize(struct vec3 vec) { float length = vec3_length_squared(vec); return length != 0.0f ? vec3_div(vec, length) : vec; }
static inline struct vec4 vec4_normalize(struct vec4 vec) { float length = vec4_length_squared(vec); return length != 0.0f ? vec4_div(vec, length) : vec; }

static inline struct vec3 vec3_cross(struct vec3 lhs, struct vec3 rhs)
{
  return vec3(
      lhs.y * rhs.z - rhs.y * lhs.z,
      lhs.z * rhs.x - rhs.z * lhs.x,
      lhs.x * rhs.y - rhs.x * lhs.y
  );
}

static inline struct mat2 mat2_zero()
{
  struct mat2 mat;
  for(unsigned i=0; i<2; ++i)
    for(unsigned j=0; j<2; ++j)
      mat.values[i][j] = 0.0f;
  return mat;
}

static inline struct mat3 mat3_zero()
{
  struct mat3 mat;
  for(unsigned i=0; i<3; ++i)
    for(unsigned j=0; j<3; ++j)
      mat.values[i][j] = 0.0f;
  return mat;
}

static inline struct mat4 mat4_zero()
{
  struct mat4 mat;
  for(unsigned i=0; i<4; ++i)
    for(unsigned j=0; j<4; ++j)
      mat.values[i][j] = 0.0f;
  return mat;
}

static inline struct mat2 mat2_identity()
{
  struct mat2 mat;
  for(unsigned i=0; i<2; ++i)
    for(unsigned j=0; j<2; ++j)
      mat.values[i][j] = (float)(i == j);
  return mat;
}

static inline struct mat3 mat3_identity()
{
  struct mat3 mat;
  for(unsigned i=0; i<3; ++i)
    for(unsigned j=0; j<3; ++j)
      mat.values[i][j] = (float)(i == j);
  return mat;
}

static inline struct mat4 mat4_identity()
{
  struct mat4 mat;
  for(unsigned i=0; i<4; ++i)
    for(unsigned j=0; j<4; ++j)
      mat.values[i][j] = (float)(i == j);
  return mat;
}

static inline struct mat2 mat2_mul(struct mat2 lhs, struct mat2 rhs)
{
  struct mat2 result = mat2_zero();
  for(unsigned i=0; i<2; ++i)
    for(unsigned j=0; j<2; ++j)
      for(unsigned k=0; k<2; ++k)
        result.values[i][j] += lhs.values[i][k] * rhs.values[k][j];
  return result;
}

static inline struct mat3 mat3_mul(struct mat3 lhs, struct mat3 rhs)
{
  struct mat3 result = mat3_zero();
  for(unsigned i=0; i<3; ++i)
    for(unsigned j=0; j<3; ++j)
      for(unsigned k=0; k<3; ++k)
        result.values[i][j] += lhs.values[i][k] * rhs.values[k][j];
  return result;
}

static inline struct mat4 mat4_mul(struct mat4 lhs, struct mat4 rhs)
{
  struct mat4 result = mat4_zero();
  for(unsigned i=0; i<4; ++i)
    for(unsigned j=0; j<4; ++j)
      for(unsigned k=0; k<4; ++k)
        result.values[i][j] += lhs.values[i][k] * rhs.values[k][j];
  return result;
}

static inline struct vec2 mat2_vmul(struct mat2 lhs, struct vec2 rhs)
{
  struct vec2 result = vec2_zero();
  for(unsigned i=0; i<2; ++i)
    for(unsigned j=0; j<2; ++j)
      result.values[i] += lhs.values[i][j] * rhs.values[j];
  return result;
}

static inline struct vec3 mat3_vmul(struct mat3 lhs, struct vec3 rhs)
{
  struct vec3 result = vec3_zero();
  for(unsigned i=0; i<3; ++i)
    for(unsigned j=0; j<3; ++j)
      result.values[i] += lhs.values[i][j] * rhs.values[j];
  return result;
}

static inline struct vec4 mat4_vmul(struct mat4 lhs, struct vec4 rhs)
{
  struct vec4 result = vec4_zero();
  for(unsigned i=0; i<4; ++i)
    for(unsigned j=0; j<4; ++j)
      result.values[i] += lhs.values[i][j] * rhs.values[j];
  return result;
}

#endif // VOXY_MATH_H
