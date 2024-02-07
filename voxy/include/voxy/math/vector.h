#ifndef VOXY_MATH_VECTOR_H
#define VOXY_MATH_VECTOR_H

#include <voxy/math/random.h>

#include <math.h>
#include <stddef.h>

/// Data-Types
typedef struct { union { float values[2]; struct { float x, y;       };                                                                   }; } fvec2_t;
typedef struct { union { float values[3]; struct { float x, y, z;    }; struct { float r, g, b;    }; struct { float yaw, pitch, roll; }; }; } fvec3_t;
typedef struct { union { float values[4]; struct { float x, y, z, w; }; struct { float r, g, b, a; };                                     }; } fvec4_t;

typedef struct { union { int values[2]; struct { int x, y;       }; }; } ivec2_t;
typedef struct { union { int values[3]; struct { int x, y, z;    }; }; } ivec3_t;
typedef struct { union { int values[4]; struct { int x, y, z, w; }; }; } ivec4_t;

/// Initializers
static inline fvec2_t fvec2(float a, float b)                   { return (fvec2_t){ .values = {a, b}       }; }
static inline fvec3_t fvec3(float a, float b, float c)          { return (fvec3_t){ .values = {a, b, c}    }; }
static inline fvec4_t fvec4(float a, float b, float c, float d) { return (fvec4_t){ .values = {a, b, c, d} }; }

static inline ivec2_t ivec2(int a, int b)               { return (ivec2_t){ .values = {a, b}       }; }
static inline ivec3_t ivec3(int a, int b, int c)        { return (ivec3_t){ .values = {a, b, c}    }; }
static inline ivec4_t ivec4(int a, int b, int c, int d) { return (ivec4_t){ .values = {a, b, c, d} }; }

/// Casts
static inline fvec2_t ivec2_as_fvec2(ivec2_t vec) { return fvec2(vec.values[0], vec.values[1]);                               }
static inline fvec3_t ivec3_as_fvec3(ivec3_t vec) { return fvec3(vec.values[0], vec.values[1], vec.values[2]);                }
static inline fvec4_t ivec4_as_fvec4(ivec4_t vec) { return fvec4(vec.values[0], vec.values[1], vec.values[2], vec.values[3]); }

static inline ivec2_t fvec2_as_ivec2_floor(fvec2_t vec) { return ivec2(floorf(vec.values[0]), floorf(vec.values[1]));                                               }
static inline ivec3_t fvec3_as_ivec3_floor(fvec3_t vec) { return ivec3(floorf(vec.values[0]), floorf(vec.values[1]), floorf(vec.values[2]));                        }
static inline ivec4_t fvec4_as_ivec4_floor(fvec4_t vec) { return ivec4(floorf(vec.values[0]), floorf(vec.values[1]), floorf(vec.values[2]), floorf(vec.values[3])); }

static inline ivec2_t fvec2_as_ivec2_ceil(fvec2_t vec) { return ivec2(ceilf(vec.values[0]), ceilf(vec.values[1]));                                               }
static inline ivec3_t fvec3_as_ivec3_ceil(fvec3_t vec) { return ivec3(ceilf(vec.values[0]), ceilf(vec.values[1]), ceilf(vec.values[2]));                        }
static inline ivec4_t fvec4_as_ivec4_ceil(fvec4_t vec) { return ivec4(ceilf(vec.values[0]), ceilf(vec.values[1]), ceilf(vec.values[2]), ceilf(vec.values[3])); }

static inline ivec2_t fvec2_as_ivec2_round(fvec2_t vec) { return ivec2(roundf(vec.values[0]), roundf(vec.values[1]));                                               }
static inline ivec3_t fvec3_as_ivec3_round(fvec3_t vec) { return ivec3(roundf(vec.values[0]), roundf(vec.values[1]), roundf(vec.values[2]));                        }
static inline ivec4_t fvec4_as_ivec4_round(fvec4_t vec) { return ivec4(roundf(vec.values[0]), roundf(vec.values[1]), roundf(vec.values[2]), roundf(vec.values[3])); }

/// Operations
#define VECTOR_TYPE(prefix, count) prefix##vec##count##_t
#define VECTOR_FUNCTION_NAME(prefix, count, name) prefix##vec##count##_##name

#define VECTOR_DEFINE_OP_NULLARY(prefix, type, count, name, op)       static inline VECTOR_TYPE(prefix, count) VECTOR_FUNCTION_NAME(prefix, count, name)()                                                                    { VECTOR_TYPE(prefix, count) result; for(unsigned i=0; i<count; ++i) result.values[i] = op;                         return result; }
#define VECTOR_DEFINE_OP_UNARY(prefix, type, count, name, op)         static inline VECTOR_TYPE(prefix, count) VECTOR_FUNCTION_NAME(prefix, count, name)         (VECTOR_TYPE(prefix, count) a)                               { VECTOR_TYPE(prefix, count) result; for(unsigned i=0; i<count; ++i) result.values[i] = op a.values[i];             return result; }
#define VECTOR_DEFINE_OP_BINARY(prefix, type, count, name, op)        static inline VECTOR_TYPE(prefix, count) VECTOR_FUNCTION_NAME(prefix, count, name)         (VECTOR_TYPE(prefix, count) a, VECTOR_TYPE(prefix, count) b) { VECTOR_TYPE(prefix, count) result; for(unsigned i=0; i<count; ++i) result.values[i] = a.values[i] op b.values[i]; return result; }
#define VECTOR_DEFINE_OP_BINARY_SCALAR(prefix, type, count, name, op) static inline VECTOR_TYPE(prefix, count) VECTOR_FUNCTION_NAME(prefix, count, name##_scalar)(VECTOR_TYPE(prefix, count) a, type b)                       { VECTOR_TYPE(prefix, count) result; for(unsigned i=0; i<count; ++i) result.values[i] = a.values[i] op b;           return result; }

#define VECTOR_DEFINE_OP_REDUCE(prefix, type, count)         static inline type                       VECTOR_FUNCTION_NAME(prefix, count, reduce)        (VECTOR_TYPE(prefix, count) a)                               { type result = 0; for(unsigned i=0; i<count; ++i) result += a.values[i]; return result; }
#define VECTOR_DEFINE_OP_DOT(prefix, type, count)            static inline type                       VECTOR_FUNCTION_NAME(prefix, count, dot)           (VECTOR_TYPE(prefix, count) a, VECTOR_TYPE(prefix, count) b) { return VECTOR_FUNCTION_NAME(prefix, count, reduce)(VECTOR_FUNCTION_NAME(prefix, count, mul)(a, b)); }
#define VECTOR_DEFINE_OP_LENGTH_SQUARED(prefix, type, count) static inline type                       VECTOR_FUNCTION_NAME(prefix, count, length_squared)(VECTOR_TYPE(prefix, count) a)                               { return VECTOR_FUNCTION_NAME(prefix, count, dot)(a, a); }
#define VECTOR_DEFINE_OP_LENGTH(prefix, type, count)         static inline type                       VECTOR_FUNCTION_NAME(prefix, count, length)        (VECTOR_TYPE(prefix, count) a)                               { return sqrt(VECTOR_FUNCTION_NAME(prefix, count, length_squared)(a)); }
#define VECTOR_DEFINE_OP_NORMALIZE(prefix, type, count)      static inline VECTOR_TYPE(prefix, count) VECTOR_FUNCTION_NAME(prefix, count, normalize)     (VECTOR_TYPE(prefix, count) a)                               { type length_squared = VECTOR_FUNCTION_NAME(prefix, count, length_squared)(a); return length_squared != 0 ? VECTOR_FUNCTION_NAME(prefix, count, div_scalar)(a, sqrt(length_squared)) : a; }

#define VECTOR_DEFINE_OP_HASH(prefix, type, count) static inline size_t VECTOR_FUNCTION_NAME(prefix, count, hash) (VECTOR_TYPE(prefix, count) a) { seed_t result = 0; for(unsigned i=0; i<count; ++i) seed_combine(&result, &a.values[i], sizeof a.values[i]); return result; }

#define VECTOR_DEFINE_OP_COPYSIGN(prefix, type, count) static inline VECTOR_TYPE(prefix, count) VECTOR_FUNCTION_NAME(prefix, count, copysign) (VECTOR_TYPE(prefix, count) a, VECTOR_TYPE(prefix, count) b) { VECTOR_TYPE(prefix, count) result; for(unsigned i=0; i<count; ++i) result.values[i] = copysign(a.values[i], b.values[i]); return result; }

#define VECTOR_DEFINE_OP_MIN(prefix, type, count) static inline VECTOR_TYPE(prefix, count) VECTOR_FUNCTION_NAME(prefix, count, min) (VECTOR_TYPE(prefix, count) a, VECTOR_TYPE(prefix, count) b) { VECTOR_TYPE(prefix, count) result; for(unsigned i=0; i<count; ++i) result.values[i] = a.values[i] < b.values[i] ? a.values[i] : b.values[i]; return result; }
#define VECTOR_DEFINE_OP_MAX(prefix, type, count) static inline VECTOR_TYPE(prefix, count) VECTOR_FUNCTION_NAME(prefix, count, max) (VECTOR_TYPE(prefix, count) a, VECTOR_TYPE(prefix, count) b) { VECTOR_TYPE(prefix, count) result; for(unsigned i=0; i<count; ++i) result.values[i] = a.values[i] > b.values[i] ? a.values[i] : b.values[i]; return result; }

#define VECTOR_DEFINE_OPS(prefix, type, count)                \
  VECTOR_DEFINE_OP_NULLARY(prefix, type, count, zero, 0)      \
  VECTOR_DEFINE_OP_UNARY  (prefix, type, count, neg, -)       \
                                                              \
  VECTOR_DEFINE_OP_BINARY(prefix, type, count, add, +)        \
  VECTOR_DEFINE_OP_BINARY(prefix, type, count, sub, -)        \
  VECTOR_DEFINE_OP_BINARY(prefix, type, count, mul, *)        \
  VECTOR_DEFINE_OP_BINARY(prefix, type, count, div, /)        \
                                                              \
  VECTOR_DEFINE_OP_BINARY_SCALAR(prefix, type, count, add, +) \
  VECTOR_DEFINE_OP_BINARY_SCALAR(prefix, type, count, sub, -) \
  VECTOR_DEFINE_OP_BINARY_SCALAR(prefix, type, count, mul, *) \
  VECTOR_DEFINE_OP_BINARY_SCALAR(prefix, type, count, div, /) \
                                                              \
  VECTOR_DEFINE_OP_REDUCE        (prefix, type, count)        \
  VECTOR_DEFINE_OP_DOT           (prefix, type, count)        \
  VECTOR_DEFINE_OP_LENGTH_SQUARED(prefix, type, count)        \
  VECTOR_DEFINE_OP_LENGTH        (prefix, type, count)        \
  VECTOR_DEFINE_OP_NORMALIZE     (prefix, type, count)        \
                                                              \
  VECTOR_DEFINE_OP_HASH(prefix, type, count)                  \
                                                              \
  VECTOR_DEFINE_OP_COPYSIGN(prefix, type, count)              \
                                                              \
  VECTOR_DEFINE_OP_MIN(prefix, type, count)                   \
  VECTOR_DEFINE_OP_MAX(prefix, type, count)

VECTOR_DEFINE_OPS(f, float, 2)
VECTOR_DEFINE_OPS(f, float, 3)
VECTOR_DEFINE_OPS(f, float, 4)

VECTOR_DEFINE_OPS(i, int, 2)
VECTOR_DEFINE_OPS(i, int, 3)
VECTOR_DEFINE_OPS(i, int, 4)

static inline fvec3_t fvec3_cross(fvec3_t lhs, fvec3_t rhs)
{
  return fvec3(
    lhs.y * rhs.z - rhs.y * lhs.z,
    lhs.z * rhs.x - rhs.z * lhs.x,
    lhs.x * rhs.y - rhs.x * lhs.y
  );
}

#endif // VOXY_MATH_VECTOR_H
