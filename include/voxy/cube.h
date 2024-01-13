#ifndef VOXY_REAL_CUBE_H
#define VOXY_REAL_CUBE_H

#include <voxy/math.h>

#include <stdint.h>

static inline void cube_emit_face(struct vec3 center, struct vec3 normal, uint32_t base, struct vec3 positions[4], struct vec3 normals[4], uint32_t indices[6])
{
  struct vec3 direction1 = vec3_dot(normal, vec3(0.0f, 0.0f, 1.0f)) == 0.0f ? vec3(0.0f, 0.0f, 1.0f) : vec3(1.0f, 0.0f, 0.0f);
  struct vec3 direction2 = vec3_cross(normal, direction1);

  positions[0] = vec3_zero();
  positions[1] = vec3_zero();
  positions[2] = vec3_zero();
  positions[3] = vec3_zero();

  positions[0] = vec3_add(positions[0], vec3_mul_s(normal, 0.5f));
  positions[1] = vec3_add(positions[1], vec3_mul_s(normal, 0.5f));
  positions[2] = vec3_add(positions[2], vec3_mul_s(normal, 0.5f));
  positions[3] = vec3_add(positions[3], vec3_mul_s(normal, 0.5f));

  positions[0] = vec3_add(positions[0], vec3_mul_s(direction2, -0.5f));
  positions[1] = vec3_add(positions[1], vec3_mul_s(direction2, -0.5f));
  positions[2] = vec3_add(positions[2], vec3_mul_s(direction2,  0.5f));
  positions[3] = vec3_add(positions[3], vec3_mul_s(direction2,  0.5f));

  positions[0] = vec3_add(positions[0], vec3_mul_s(direction1, -0.5f));
  positions[1] = vec3_add(positions[1], vec3_mul_s(direction1,  0.5f));
  positions[2] = vec3_add(positions[2], vec3_mul_s(direction1, -0.5f));
  positions[3] = vec3_add(positions[3], vec3_mul_s(direction1,  0.5f));

  positions[0] = vec3_add(center, positions[0]);
  positions[1] = vec3_add(center, positions[1]);
  positions[2] = vec3_add(center, positions[2]);
  positions[3] = vec3_add(center, positions[3]);

  normals[0] = normal;
  normals[1] = normal;
  normals[2] = normal;
  normals[3] = normal;

  indices[0] = base + 0;
  indices[1] = base + 1;
  indices[2] = base + 2;
  indices[3] = base + 2;
  indices[4] = base + 1;
  indices[5] = base + 3;
}

static inline void cube_emit(struct vec3 center, uint32_t base, struct vec3 positions[24], struct vec3 normals[24], uint32_t indices[36])
{
  cube_emit_face(center, vec3(-1.0f,  0.0f,  0.0f), base + 0,  &positions[0],  &normals[0],  &indices[0]);
  cube_emit_face(center, vec3( 1.0f,  0.0f,  0.0f), base + 4,  &positions[4],  &normals[4],  &indices[6]);
  cube_emit_face(center, vec3( 0.0f, -1.0f,  0.0f), base + 8,  &positions[8],  &normals[8],  &indices[12]);
  cube_emit_face(center, vec3( 0.0f,  1.0f,  0.0f), base + 12, &positions[12], &normals[12], &indices[18]);
  cube_emit_face(center, vec3( 0.0f,  0.0f, -1.0f), base + 16, &positions[16], &normals[16], &indices[24]);
  cube_emit_face(center, vec3( 0.0f,  0.0f,  1.0f), base + 20, &positions[20], &normals[20], &indices[30]);
}

#endif // VOXY_REAL_CUBE_H
