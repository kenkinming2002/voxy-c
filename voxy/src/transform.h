#ifndef VOXY_TRANSFORM_H
#define VOXY_TRANSFORM_H

#include <voxy/math/vector.h>
#include <voxy/math/matrix.h>

fmat4_t fmat4_translate(fvec3_t translation);
fmat4_t fmat4_translate_inverse(fvec3_t translation);

fmat4_t fmat4_rotate_x(float angle);
fmat4_t fmat4_rotate_y(float angle);
fmat4_t fmat4_rotate_z(float angle);

fmat4_t fmat4_rotate(fvec3_t rotation);
fmat4_t fmat4_rotate_inverse(fvec3_t rotation);

struct transform
{
  fvec3_t translation;
  fvec3_t rotation; // Euler angle
};

fmat4_t transform_matrix(struct transform *transform);
fmat4_t transform_matrix_inverse(struct transform *transform);

fvec3_t transform_right  (struct transform *transform);
fvec3_t transform_forward(struct transform *transform);
fvec3_t transform_up     (struct transform *transform);

void transform_rotate(struct transform *transform, fvec3_t rotation);
void transform_local_translate(struct transform *transform, fvec3_t translation);

#endif // VOXY_TRANSFORM_H
