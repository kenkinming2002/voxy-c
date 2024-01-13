#ifndef VOXY_MATH_H
#define VOXY_MATH_H

#include <math.h>

/*********
 * Types *
 *********/

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

/***********************
 * Vector Initializers *
 ***********************/

static inline struct vec2 vec2(float a, float b)                   { return (struct vec2){ .values = {a, b}       }; }
static inline struct vec3 vec3(float a, float b, float c)          { return (struct vec3){ .values = {a, b, c}    }; }
static inline struct vec4 vec4(float a, float b, float c, float d) { return (struct vec4){ .values = {a, b, c, d} }; }

static inline struct vec2 vec2_zero() { return vec2(0.0f, 0.0f);             }
static inline struct vec3 vec3_zero() { return vec3(0.0f, 0.0f, 0.0f);       }
static inline struct vec4 vec4_zero() { return vec4(0.0f, 0.0f, 0.0f, 0.0f); }

/***********************
 * Matrix Initializers *
 ***********************/

static inline struct mat2 mat2(float a, float b, float c, float d)                                                                                                             { return (struct mat2){ .values = {{a, b}, {c, d}} }; }
static inline struct mat3 mat3(float a, float b, float c, float d, float e, float f, float g, float h, float i)                                                                { return (struct mat3){ .values = {{a, b, c}, {d, e, f}, {g, h, i}} }; }
static inline struct mat4 mat4(float a, float b, float c, float d, float e, float f, float g, float h, float i, float j, float k, float l, float m, float n, float o, float p) { return (struct mat4){ .values = {{a, b, c, d}, {e, f, g, h}, {i, j, k, l}, {m, n, o, p}} }; }

static inline struct mat2 mat2_zero() { return mat2(0.0f, 0.0f, 0.0f, 0.0f);                                                                         }
static inline struct mat3 mat3_zero() { return mat3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);                                           }
static inline struct mat4 mat4_zero() { return mat4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f); }

static inline struct mat2 mat2_identity() { return mat2(1.0f, 0.0f, 0.0f, 1.0f);                                                                         }
static inline struct mat3 mat3_identity() { return mat3(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);                                           }
static inline struct mat4 mat4_identity() { return mat4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f); }

/********************
 * Vector Functions *
 ********************/

#define VEC_DEFINE_BINARY_OP(count, name, op)                                                   \
  static inline struct vec##count vec##count##_##name(struct vec##count a, struct vec##count b) \
  {                                                                                             \
    struct vec##count result;                                                                   \
    for(unsigned i=0; i<count; ++i)                                                             \
      result.values[i] = a.values[i] op b.values[i];                                            \
    return result;                                                                              \
  }

#define VEC_DEFINE_BINARY_OP_SCALAR(count, name, op)                                    \
  static inline struct vec##count vec##count##_##name##_s(struct vec##count a, float b) \
  {                                                                                     \
    struct vec##count result;                                                           \
    for(unsigned i=0; i<count; ++i)                                                     \
      result.values[i] = a.values[i] op b;                                              \
    return result;                                                                      \
  }

#define VEC_DEFINE_OP_NEG(count)                                        \
  static inline struct vec##count vec##count##_neg(struct vec##count a) \
  {                                                                     \
    struct vec##count result;                                           \
    for(unsigned i=0; i<count; ++i)                                     \
      result.values[i] = -a.values[i];                                  \
    return result;                                                      \
  }

#define VEC_DEFINE_OP_REDUCE(count)                            \
  static inline float vec##count##_reduce(struct vec##count a) \
  {                                                            \
    float result = 0.0f;                                       \
    for(unsigned i=0; i<count; ++i)                            \
      result += a.values[i];                                   \
    return result;                                             \
  }

#define VEC_DEFINE_OP_DOT(count)                                                 \
  static inline float vec##count##_dot(struct vec##count a, struct vec##count b) \
  {                                                                              \
    return vec##count##_reduce(vec##count##_mul(a, b));                          \
  }

#define VEC_DEFINE_OP_LENGTH_SQUARED(count)                            \
  static inline float vec##count##_length_squared(struct vec##count a) \
  {                                                                    \
    return vec##count##_dot(a, a);                                     \
  }

#define VEC_DEFINE_OP_LENGTH(count)                            \
  static inline float vec##count##_length(struct vec##count a) \
  {                                                            \
    return sqrtf(vec##count##_length_squared(a));              \
  }

#define VEC_DEFINE_OP_NORMALIZE(count)                                        \
  static inline struct vec##count vec##count##_normalize(struct vec##count a) \
  {                                                                           \
    float length_squared = vec##count##_length_squared(a);                    \
    if(length_squared == 0.0f)                                                \
      return a;                                                               \
    return vec##count##_div_s(a, vec##count##_length(a));                     \
  }

#define VEC_DEFINE_OPS(count) \
  VEC_DEFINE_BINARY_OP(count, add, +)        \
  VEC_DEFINE_BINARY_OP(count, sub, -)        \
  VEC_DEFINE_BINARY_OP(count, mul, *)        \
  VEC_DEFINE_BINARY_OP(count, div, /)        \
  VEC_DEFINE_BINARY_OP_SCALAR(count, add, +) \
  VEC_DEFINE_BINARY_OP_SCALAR(count, sub, -) \
  VEC_DEFINE_BINARY_OP_SCALAR(count, mul, *) \
  VEC_DEFINE_BINARY_OP_SCALAR(count, div, /) \
  VEC_DEFINE_OP_NEG(count)                   \
  VEC_DEFINE_OP_REDUCE(count)                \
  VEC_DEFINE_OP_DOT(count)                   \
  VEC_DEFINE_OP_LENGTH_SQUARED(count)        \
  VEC_DEFINE_OP_LENGTH(count)                \
  VEC_DEFINE_OP_NORMALIZE(count)

VEC_DEFINE_OPS(2)
VEC_DEFINE_OPS(3)
VEC_DEFINE_OPS(4)

#undef VEC_DEFINE_BINARY_OP
#undef VEC_DEFINE_BINARY_OP
#undef VEC_DEFINE_BINARY_OP
#undef VEC_DEFINE_BINARY_OP
#undef VEC_DEFINE_BINARY_OP_SCALAR
#undef VEC_DEFINE_BINARY_OP_SCALAR
#undef VEC_DEFINE_BINARY_OP_SCALAR
#undef VEC_DEFINE_BINARY_OP_SCALAR
#undef VEC_DEFINE_OP_NEG
#undef VEC_DEFINE_OP_REDUCE
#undef VEC_DEFINE_OP_DOT
#undef VEC_DEFINE_OP_LENGTH_SQUARED
#undef VEC_DEFINE_OP_LENGTH
#undef VEC_DEFINE_OP_NORMALIZE
#undef VEC_DEFINE_OPS

static inline struct vec3 vec3_cross(struct vec3 lhs, struct vec3 rhs)
{
  return vec3(
      lhs.y * rhs.z - rhs.y * lhs.z,
      lhs.z * rhs.x - rhs.z * lhs.x,
      lhs.x * rhs.y - rhs.x * lhs.y
  );
}

/********************
 * Matrix Functions *
 ********************/
#define MAT_DEFINE_OP_MUL(count)                                                                 \
  static inline struct mat##count mat##count##_mul(struct mat##count lhs, struct mat##count rhs) \
  {                                                                                              \
    struct mat##count result = mat##count##_zero();                                              \
    for(unsigned i=0; i<count; ++i)                                                              \
      for(unsigned j=0; j<count; ++j)                                                            \
        for(unsigned k=0; k<count; ++k)                                                          \
          result.values[i][j] += lhs.values[i][k] * rhs.values[k][j];                            \
    return result;                                                                               \
  }

#define MAT_DEFINE_OP_MUL_V(count)                                                                 \
  static inline struct vec##count mat##count##_mul_v(struct mat##count lhs, struct vec##count rhs) \
  {                                                                                                \
    struct vec##count result = vec##count##_zero();                                                \
    for(unsigned i=0; i<count; ++i)                                                                \
      for(unsigned j=0; j<count; ++j)                                                              \
          result.values[i] += lhs.values[i][j] * rhs.values[j];                                    \
    return result;                                                                                 \
  }

#define MAT_DEFINE_OPS(count) \
  MAT_DEFINE_OP_MUL(count)    \
  MAT_DEFINE_OP_MUL_V(count)

MAT_DEFINE_OPS(2)
MAT_DEFINE_OPS(3)
MAT_DEFINE_OPS(4)

#undef MAT_DEFINE_OP_MUL
#undef MAT_DEFINE_OP_MUL_V
#undef MAT_DEFINE_OPS

#endif // VOXY_MATH_H
