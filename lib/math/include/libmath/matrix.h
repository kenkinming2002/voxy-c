#ifndef LIBMATH_MATRIX_H
#define LIBMATH_MATRIX_H

#include <libmath/vector.h>

typedef struct { float values[2][2]; } fmat2_t;
typedef struct { float values[3][3]; } fmat3_t;
typedef struct { float values[4][4]; } fmat4_t;

/// Initializers
static inline fmat2_t fmat2(float a, float b, float c, float d)                                                                                                             { return (fmat2_t){ .values = {{a, b}, {c, d}} }; }
static inline fmat3_t fmat3(float a, float b, float c, float d, float e, float f, float g, float h, float i)                                                                { return (fmat3_t){ .values = {{a, b, c}, {d, e, f}, {g, h, i}} }; }
static inline fmat4_t fmat4(float a, float b, float c, float d, float e, float f, float g, float h, float i, float j, float k, float l, float m, float n, float o, float p) { return (fmat4_t){ .values = {{a, b, c, d}, {e, f, g, h}, {i, j, k, l}, {m, n, o, p}} }; }

static inline fmat2_t fmat2_zero() { return fmat2(0.0f, 0.0f, 0.0f, 0.0f);                                                                         }
static inline fmat3_t fmat3_zero() { return fmat3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);                                           }
static inline fmat4_t fmat4_zero() { return fmat4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f); }

static inline fmat2_t fmat2_identity() { return fmat2(1.0f, 0.0f, 0.0f, 1.0f);                                                                         }
static inline fmat3_t fmat3_identity() { return fmat3(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);                                           }
static inline fmat4_t fmat4_identity() { return fmat4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f); }

/// Operations
#define MATRIX_TYPE(prefix, count) prefix##mat##count##_t
#define MATRIX_FUNCTION_NAME(prefix, count, name) prefix##mat##count##_##name

#define MATRIX_DEFINE_OP_MUL(prefix, count)                                                                                                         \
  static inline MATRIX_TYPE(prefix, count) MATRIX_FUNCTION_NAME(prefix, count, mul)(MATRIX_TYPE(prefix, count) lhs, MATRIX_TYPE(prefix, count) rhs) \
  {                                                                                                                                                 \
    MATRIX_TYPE(prefix, count) result = MATRIX_FUNCTION_NAME(prefix, count, zero)();                                                                \
    for(unsigned i=0; i<count; ++i)                                                                                                                 \
      for(unsigned j=0; j<count; ++j)                                                                                                               \
        for(unsigned k=0; k<count; ++k)                                                                                                             \
          result.values[i][j] += lhs.values[i][k] * rhs.values[k][j];                                                                               \
    return result;                                                                                                                                  \
  }

#define MATRIX_DEFINE_OP_MUL_VECTOR(prefix, count)                                                                                                      \
  static inline VECTOR_TYPE(prefix, count) MATRIX_FUNCTION_NAME(prefix, count, mul_vec)(MATRIX_TYPE(prefix, count) lhs, VECTOR_TYPE(prefix, count) rhs) \
  {                                                                                                                                                     \
    VECTOR_TYPE(prefix, count) result = VECTOR_FUNCTION_NAME(prefix, count, zero)();                                                                    \
    for(unsigned i=0; i<count; ++i)                                                                                                                     \
      for(unsigned j=0; j<count; ++j)                                                                                                                   \
          result.values[i] += lhs.values[i][j] * rhs.values[j];                                                                                         \
    return result;                                                                                                                                      \
  }

#define MATRIX_DEFINE_OPS(prefix, count) \
  MATRIX_DEFINE_OP_MUL(prefix, count)    \
  MATRIX_DEFINE_OP_MUL_VECTOR(prefix, count)

MATRIX_DEFINE_OPS(f, 2)
MATRIX_DEFINE_OPS(f, 3)
MATRIX_DEFINE_OPS(f, 4)

#endif // LIBMATH_MATRIX_H
