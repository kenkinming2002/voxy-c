#include <voxy/math.h>
#include <stdio.h>

static struct vec3 vertices[24];
static unsigned    vertex_count;

static unsigned indices[36];
static unsigned index_count;

static void emit_face(struct vec3 normal)
{
  struct vec3 center = vec3_add(vec3(0.5f, 0.5f, 0.5f), vec3_mul(normal, 0.5f));

  struct vec3 direction1 = vec3_dot(normal, vec3(0.0f, 0.0f, 1.0f)) == 0.0f ? vec3(0.0f, 0.0f, 1.0f) : vec3(1.0f, 0.0f, 0.0f);
  struct vec3 direction2 = vec3_cross(normal, direction1);

  vertices[vertex_count + 0] = vec3_add(center, vec3_add(vec3_mul(direction2, -0.5f), vec3_mul(direction1, -0.5f)));
  vertices[vertex_count + 1] = vec3_add(center, vec3_add(vec3_mul(direction2, -0.5f), vec3_mul(direction1,  0.5f)));
  vertices[vertex_count + 2] = vec3_add(center, vec3_add(vec3_mul(direction2,  0.5f), vec3_mul(direction1, -0.5f)));
  vertices[vertex_count + 3] = vec3_add(center, vec3_add(vec3_mul(direction2,  0.5f), vec3_mul(direction1,  0.5f)));

  indices[index_count + 0] = vertex_count + 0;
  indices[index_count + 1] = vertex_count + 1;
  indices[index_count + 2] = vertex_count + 2;
  indices[index_count + 3] = vertex_count + 2;
  indices[index_count + 4] = vertex_count + 1;
  indices[index_count + 5] = vertex_count + 3;

  vertex_count += 4;
  index_count  += 6;
}

int main()
{
  emit_face(vec3(-1.0f,  0.0f,  0.0f));
  emit_face(vec3( 1.0f,  0.0f,  0.0f));
  emit_face(vec3( 0.0f, -1.0f,  0.0f));
  emit_face(vec3( 0.0f,  1.0f,  0.0f));
  emit_face(vec3( 0.0f,  0.0f, -1.0f));
  emit_face(vec3( 0.0f,  0.0f,  1.0f));

  for(unsigned i=0; i<vertex_count; ++i)
    printf("vertex = %.1ff, %.1ff, %.1ff\n", vertices[i].x, vertices[i].y, vertices[i].z);

  for(unsigned i=0; i<index_count; ++i)
    printf("index = %u\n", indices[i]);
}
