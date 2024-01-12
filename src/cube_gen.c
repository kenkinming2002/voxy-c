#include <voxy/math.h>
#include <stdio.h>

static unsigned vertex_count;
static struct vec3 positions[24];
static struct vec3 normals[24];

static unsigned indices[36];
static unsigned index_count;

static void emit_face(struct vec3 normal)
{
  struct vec3 center = vec3_add(vec3(0.5f, 0.5f, 0.5f), vec3_mul(normal, 0.5f));

  struct vec3 direction1 = vec3_dot(normal, vec3(0.0f, 0.0f, 1.0f)) == 0.0f ? vec3(0.0f, 0.0f, 1.0f) : vec3(1.0f, 0.0f, 0.0f);
  struct vec3 direction2 = vec3_cross(normal, direction1);

  positions[vertex_count + 0] = vec3_add(center, vec3_add(vec3_mul(direction2, -0.5f), vec3_mul(direction1, -0.5f)));
  positions[vertex_count + 1] = vec3_add(center, vec3_add(vec3_mul(direction2, -0.5f), vec3_mul(direction1,  0.5f)));
  positions[vertex_count + 2] = vec3_add(center, vec3_add(vec3_mul(direction2,  0.5f), vec3_mul(direction1, -0.5f)));
  positions[vertex_count + 3] = vec3_add(center, vec3_add(vec3_mul(direction2,  0.5f), vec3_mul(direction1,  0.5f)));

  normals[vertex_count + 0] = normal;
  normals[vertex_count + 1] = normal;
  normals[vertex_count + 2] = normal;
  normals[vertex_count + 3] = normal;

  indices[index_count + 0] = vertex_count + 0;
  indices[index_count + 1] = vertex_count + 1;
  indices[index_count + 2] = vertex_count + 2;
  indices[index_count + 3] = vertex_count + 2;
  indices[index_count + 4] = vertex_count + 1;
  indices[index_count + 5] = vertex_count + 3;

  vertex_count += 4;
  index_count  += 6;
}

static void emit_cube()
{
  emit_face(vec3(-1.0f,  0.0f,  0.0f));
  emit_face(vec3( 1.0f,  0.0f,  0.0f));
  emit_face(vec3( 0.0f, -1.0f,  0.0f));
  emit_face(vec3( 0.0f,  1.0f,  0.0f));
  emit_face(vec3( 0.0f,  0.0f, -1.0f));
  emit_face(vec3( 0.0f,  0.0f,  1.0f));
}

int main()
{
  emit_cube();

  printf("#ifndef VOXY_CUBE_H\n");
  printf("#define VOXY_CUBE_H\n");
  printf("#include <voxy/math.h>\n");
  printf("\n");

  printf("static const struct vec3 CUBE_POSITIONS[] = {\n");
  for(unsigned i=0; i<vertex_count; ++i)
    printf("  (struct vec3){ .x = %ff, .y = %ff, .z = %ff },\n", positions[i].x, positions[i].y, positions[i].z);
  printf("};\n");
  printf("\n");

  printf("static const struct vec3 CUBE_NORMALS[] = {\n");
  for(unsigned i=0; i<vertex_count; ++i)
    printf("  (struct vec3){ .x = %ff, .y = %ff, .z = %ff },\n", normals[i].x, normals[i].y, normals[i].z);
  printf("};\n");
  printf("\n");

  printf("static const unsigned CUBE_INDICES[] = {\n");
  for(unsigned i=0; i<index_count; ++i)
    printf("  %u,\n", indices[i]);
  printf("};\n");
  printf("\n");

  printf("#endif // VOXY_CUBE_H\n");
}
