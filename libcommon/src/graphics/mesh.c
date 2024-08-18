#include <libcommon/graphics/mesh.h>

#include <libcommon/core/log.h>
#include <libcommon/math/vector.h>
#include <libcommon/utils/dynamic_array.h>

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int mesh_init(struct mesh *mesh)
{
  glGenVertexArrays(1, &mesh->vao);
  glBindVertexArray(mesh->vao);

  glGenBuffers(1, &mesh->vbo);
  glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);

  glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct mesh_vertex), (void *)offsetof(struct mesh_vertex, position));
  glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct mesh_vertex), (void *)offsetof(struct mesh_vertex, normal));
  glEnableVertexAttribArray(2); glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(struct mesh_vertex), (void *)offsetof(struct mesh_vertex, uv));

  glGenBuffers(1, &mesh->vbo_instanced);
  glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo_instanced);

  glEnableVertexAttribArray(3); glVertexAttribDivisor(3, 1); glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(struct mesh_vertex_instanced), (void *)(offsetof(struct mesh_vertex_instanced, transform) + 0  * sizeof(float)));
  glEnableVertexAttribArray(4); glVertexAttribDivisor(4, 1); glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(struct mesh_vertex_instanced), (void *)(offsetof(struct mesh_vertex_instanced, transform) + 4  * sizeof(float)));
  glEnableVertexAttribArray(5); glVertexAttribDivisor(5, 1); glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(struct mesh_vertex_instanced), (void *)(offsetof(struct mesh_vertex_instanced, transform) + 8  * sizeof(float)));
  glEnableVertexAttribArray(6); glVertexAttribDivisor(6, 1); glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(struct mesh_vertex_instanced), (void *)(offsetof(struct mesh_vertex_instanced, transform) + 12 * sizeof(float)));
  glEnableVertexAttribArray(7); glVertexAttribDivisor(7, 1); glVertexAttribPointer(7, 1, GL_FLOAT, GL_FALSE, sizeof(struct mesh_vertex_instanced), (void *)offsetof(struct mesh_vertex_instanced, light));

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  return 0;
}

void mesh_fini(struct mesh *mesh)
{
  glDeleteBuffers(1, &mesh->vbo);
  glDeleteBuffers(1, &mesh->vbo_instanced);
  glDeleteVertexArrays(1, &mesh->vao);
}

void mesh_update(struct mesh *mesh, const struct mesh_vertex *vertices, size_t vertex_count)
{
  glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
  glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof *vertices, vertices, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  mesh->count = vertex_count;
}

void mesh_update_instanced(const struct mesh *mesh, const struct mesh_vertex_instanced *vertices, size_t vertex_count)
{
  glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo_instanced);
  glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof *vertices, vertices, GL_STREAM_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

int mesh_load(struct mesh *mesh, const char *filepath)
{
  FILE *f = fopen(filepath, "r");
  if(!f)
  {
    LOG_ERROR("Failed to open file %s", filepath);
    return -1;
  }

  int result = 0;

  DYNAMIC_ARRAY_DECLARE(vertices, struct mesh_vertex);
  DYNAMIC_ARRAY_DECLARE(positions, fvec3_t);
  DYNAMIC_ARRAY_DECLARE(normals, fvec3_t);
  DYNAMIC_ARRAY_DECLARE(uvs, fvec2_t);

  char buffer[1024];
  char *line;
  while((line = fgets(buffer, sizeof buffer, f)))
  {
    char c;

    fvec3_t position;
    if(sscanf(line, "v %f %f %f %c", &position.x, &position.y, &position.z, &c) == 3)
    {
      DYNAMIC_ARRAY_APPEND(positions, position);
      continue;
    }

    fvec3_t normal;
    if(sscanf(line, "vn %f %f %f %c", &normal.x, &normal.y, &normal.z, &c) == 3)
    {
      DYNAMIC_ARRAY_APPEND(normals, normal);
      continue;
    }

    fvec2_t uv;
    if(sscanf(line, "vt %f %f %c", &uv.x, &uv.y, &c) == 2)
    {
      DYNAMIC_ARRAY_APPEND(uvs, uv);
      continue;
    }

    size_t position_indices[3];
    size_t normal_indices[3];
    size_t uv_indices[3];
    if(sscanf(line, "f %zu/%zu/%zu %zu/%zu/%zu %zu/%zu/%zu %c",
          &position_indices[0], &uv_indices[0], &normal_indices[0],
          &position_indices[1], &uv_indices[1], &normal_indices[1],
          &position_indices[2], &uv_indices[2], &normal_indices[2],
          &c) == 9)
    {

      for(size_t j=0; j<3; ++j)
      {
        if(position_indices[j] >= positions.item_count + 1)
        {
          LOG_ERROR("Out of bound position index(1-based): %zu in array of length %zu", position_indices[j], positions.item_count);
          result = -1;
          goto out;
        }

        if(normal_indices[j] >= normals.item_count + 1)
        {
          LOG_ERROR("Out of bound normal index(1-based): %zu in array of length %zu", normal_indices[j], normals.item_count);
          result = -1;
          goto out;
        }

        if(uv_indices[j] >= uvs.item_count + 1)
        {
          LOG_ERROR("Out of bound uv index(1-based): %zu in array of length %zu", uv_indices[j], uvs.item_count);
          result = -1;
          goto out;
        }

        struct mesh_vertex vertex;
        vertex.position = positions.items[position_indices[j]-1];
        vertex.normal   = normals  .items[normal_indices  [j]-1];
        vertex.uv       = uvs      .items[uv_indices      [j]-1];
        DYNAMIC_ARRAY_APPEND(vertices, vertex);
      }
      continue;
    }

    if(line[0] == '#')
      continue;

    if(sscanf(line, "s %c", &c) == 1) continue;
    if(sscanf(line, "o %c", &c) == 1) continue;
    if(sscanf(line, "mtllib %c", &c) == 1) continue;
    if(sscanf(line, "usemtl %c", &c) == 1) continue;

    LOG_ERROR("Unknown line: %s", line);
    result = -1;
    goto out;
  }

  mesh_update(mesh, vertices.items, vertices.item_count);

out:
  free(positions.items);
  free(normals.items);
  free(uvs.items);
  free(vertices.items);
  return result;
}

