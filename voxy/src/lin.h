#ifndef VOXY_LIN_H
#define VOXY_LIN_H

#include "random.h"

#include <math.h>

/*********
 * Types *
 *********/
struct ivec2
{
  union
  {
    int values[2];
    struct { int x, y; };
  };
};

struct ivec3
{
  union
  {
    int values[3];
    struct { int x, y, z; };
  };
};

struct ivec4
{
  union
  {
    int values[4];
    struct { int x, y, z, w; };
  };
};

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

static inline struct ivec2 ivec2(int a, int b)               { return (struct ivec2){ .values = {a, b}       }; }
static inline struct ivec3 ivec3(int a, int b, int c)        { return (struct ivec3){ .values = {a, b, c}    }; }
static inline struct ivec4 ivec4(int a, int b, int c, int d) { return (struct ivec4){ .values = {a, b, c, d} }; }

static inline struct ivec2 ivec2_zero() { return ivec2(0, 0);       }
static inline struct ivec3 ivec3_zero() { return ivec3(0, 0, 0);    }
static inline struct ivec4 ivec4_zero() { return ivec4(0, 0, 0, 0); }

static inline struct vec2 ivec2_as_vec2(struct ivec2 vec) { return vec2(vec.values[0], vec.values[1]);                               }
static inline struct vec3 ivec3_as_vec3(struct ivec3 vec) { return vec3(vec.values[0], vec.values[1], vec.values[2]);                }
static inline struct vec4 ivec4_as_vec4(struct ivec4 vec) { return vec4(vec.values[0], vec.values[1], vec.values[2], vec.values[3]); }

static inline struct ivec2 vec2_as_ivec2_floor(struct vec2 vec) { return ivec2(floorf(vec.values[0]), floorf(vec.values[1]));                                               }
static inline struct ivec3 vec3_as_ivec3_floor(struct vec3 vec) { return ivec3(floorf(vec.values[0]), floorf(vec.values[1]), floorf(vec.values[2]));                        }
static inline struct ivec4 vec4_as_ivec4_floor(struct vec4 vec) { return ivec4(floorf(vec.values[0]), floorf(vec.values[1]), floorf(vec.values[2]), floorf(vec.values[3])); }

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

#define VEC_DEFINE_BINARY_OP(type, count, prefix, name, op)                                                   \
  static inline struct prefix##vec##count prefix##vec##count##_##name(struct prefix##vec##count a, struct prefix##vec##count b) \
  {                                                                                             \
    struct prefix##vec##count result;                                                                   \
    for(unsigned i=0; i<count; ++i)                                                             \
      result.values[i] = a.values[i] op b.values[i];                                            \
    return result;                                                                              \
  }

#define VEC_DEFINE_BINARY_OP_SCALAR(type, count, prefix, name, op)                                    \
  static inline struct prefix##vec##count prefix##vec##count##_##name##_s(struct prefix##vec##count a, type b) \
  {                                                                                     \
    struct prefix##vec##count result;                                                           \
    for(unsigned i=0; i<count; ++i)                                                     \
      result.values[i] = a.values[i] op b;                                              \
    return result;                                                                      \
  }

#define VEC_DEFINE_OP_NEG(type, count, prefix)                                        \
  static inline struct prefix##vec##count prefix##vec##count##_neg(struct prefix##vec##count a) \
  {                                                                     \
    struct prefix##vec##count result;                                           \
    for(unsigned i=0; i<count; ++i)                                     \
      result.values[i] = -a.values[i];                                  \
    return result;                                                      \
  }

#define VEC_DEFINE_OP_REDUCE(type, count, prefix)                            \
  static inline type prefix##vec##count##_reduce(struct prefix##vec##count a) \
  {                                                            \
    type result = 0;                                       \
    for(unsigned i=0; i<count; ++i)                            \
      result += a.values[i];                                   \
    return result;                                             \
  }

#define VEC_DEFINE_OP_DOT(type, count, prefix)                                                 \
  static inline type prefix##vec##count##_dot(struct prefix##vec##count a, struct prefix##vec##count b) \
  {                                                                              \
    return prefix##vec##count##_reduce(prefix##vec##count##_mul(a, b));                          \
  }

#define VEC_DEFINE_OP_LENGTH_SQUARED(type, count, prefix)                            \
  static inline type prefix##vec##count##_length_squared(struct prefix##vec##count a) \
  {                                                                    \
    return prefix##vec##count##_dot(a, a);                                     \
  }

#define VEC_DEFINE_OP_LENGTH(type, count, prefix)                            \
  static inline type prefix##vec##count##_length(struct prefix##vec##count a) \
  {                                                            \
    return sqrtf(prefix##vec##count##_length_squared(a));              \
  }

#define VEC_DEFINE_OP_NORMALIZE(type, count, prefix)                                        \
  static inline struct prefix##vec##count prefix##vec##count##_normalize(struct prefix##vec##count a) \
  {                                                                           \
    type length_squared = prefix##vec##count##_length_squared(a);                    \
    if(length_squared == 0)                                                \
      return a;                                                               \
    return prefix##vec##count##_div_s(a, prefix##vec##count##_length(a));                     \
  }

#define VEC_DEFINE_OP_HASH(type, count, prefix)                               \
  static inline seed_t prefix##vec##count##_hash(struct prefix##vec##count a) \
  {                                                                           \
    seed_t seed = 0x1313897312983;                                            \
    for(unsigned i=0; i<count; ++i)                                           \
      seed_combine(&seed, &a.values[i], sizeof a.values[i]);                  \
    return seed;                                                              \
  }

#define VEC_DEFINE_OPS(type, count, prefix) \
  VEC_DEFINE_BINARY_OP(type, count, prefix, add, +)        \
  VEC_DEFINE_BINARY_OP(type, count, prefix, sub, -)        \
  VEC_DEFINE_BINARY_OP(type, count, prefix, mul, *)        \
  VEC_DEFINE_BINARY_OP(type, count, prefix, div, /)        \
  VEC_DEFINE_BINARY_OP_SCALAR(type, count, prefix, add, +) \
  VEC_DEFINE_BINARY_OP_SCALAR(type, count, prefix, sub, -) \
  VEC_DEFINE_BINARY_OP_SCALAR(type, count, prefix, mul, *) \
  VEC_DEFINE_BINARY_OP_SCALAR(type, count, prefix, div, /) \
  VEC_DEFINE_OP_NEG(type, count, prefix)                   \
  VEC_DEFINE_OP_REDUCE(type, count, prefix)                \
  VEC_DEFINE_OP_DOT(type, count, prefix)                   \
  VEC_DEFINE_OP_LENGTH_SQUARED(type, count, prefix)        \
  VEC_DEFINE_OP_LENGTH(type, count, prefix)                \
  VEC_DEFINE_OP_NORMALIZE(type, count, prefix)             \
  VEC_DEFINE_OP_HASH(type, count, prefix)

VEC_DEFINE_OPS(float, 2,)
VEC_DEFINE_OPS(float, 3,)
VEC_DEFINE_OPS(float, 4,)

VEC_DEFINE_OPS(int, 2, i)
VEC_DEFINE_OPS(int, 3, i)
VEC_DEFINE_OPS(int, 4, i)

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

#endif // VOXY_LIN_H
