#include <voxy/graphics/mesh.h>

#include <voxy/core/log.h>
#include <voxy/dynamic_array.h>
#include <voxy/math/vector.h>

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int mesh_load(struct mesh *mesh, const char *filepath)
{
  FILE *f = fopen(filepath, "r");
  if(!f)
  {
    LOG_ERROR("Failed to open file %s", filepath);
    return -1;
  }

  int result = 0;

  // We make the simplifying assumption that our mesh are formed from vertices
  // of 3d position, 3d normal and 2d texture coordinates.
  struct vertex
  {
    fvec3_t position;
    fvec3_t normal;
    fvec2_t uv;
  };

  DYNAMIC_ARRAY_DECLARE(vertices, struct vertex);

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

        struct vertex vertex;
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

  glGenVertexArrays(1, &mesh->vao);
  glBindVertexArray(mesh->vao);

  glGenBuffers(1, &mesh->vbo);
  glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.item_count * sizeof *vertices.items, vertices.items, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (void *)offsetof(struct vertex, position));
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (void *)offsetof(struct vertex, normal));
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (void *)offsetof(struct vertex, uv));

  mesh->count = vertices.item_count;

out:
  free(positions.items);
  free(normals.items);
  free(uvs.items);
  free(vertices.items);
  return result;
}

void mesh_unload(struct mesh *mesh)
{
  glDeleteBuffers(1, &mesh->vbo);
  glDeleteVertexArrays(1, &mesh->vao);
}
