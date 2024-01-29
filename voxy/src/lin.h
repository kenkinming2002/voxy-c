#ifndef VOXY_LIN_H
#define VOXY_LIN_H

#include "random.h"
#include "vector.h"

#include <math.h>

/*********
 * Types *
 *********/
struct mat2 { float values[2][2]; };
struct mat3 { float values[3][3]; };
struct mat4 { float values[4][4]; };

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

#define MAT_DEFINE_OP_MUL_V(count)                                                             \
  static inline fvec##count##_t mat##count##_mul_v(struct mat##count lhs, fvec##count##_t rhs) \
  {                                                                                            \
    fvec##count##_t result = fvec##count##_zero();                                             \
    for(unsigned i=0; i<count; ++i)                                                            \
      for(unsigned j=0; j<count; ++j)                                                          \
          result.values[i] += lhs.values[i][j] * rhs.values[j];                                \
    return result;                                                                             \
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
