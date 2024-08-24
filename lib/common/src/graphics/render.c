#include <libcommon/graphics/render.h>

#include <libcommon/core/log.h>
#include <libcommon/utils/dynamic_array.h>

struct renderable
{
  const struct camera *camera;
  const struct gl_texture_2d *texture;
  const struct mesh *mesh;

  transform_t transform;
  float light;
};

static DYNAMIC_ARRAY_DECLARE(renderables, struct renderable);

void render(const struct camera *camera, const struct mesh *mesh, const struct gl_texture_2d *texture, transform_t transform, float light)
{
  struct renderable renderable;
  renderable.camera = camera;
  renderable.mesh = mesh;
  renderable.texture = texture;
  renderable.transform = transform;
  renderable.light = light;
  DYNAMIC_ARRAY_APPEND(renderables, renderable);
}

static int compar(const void *item1, const void *item2)
{
  const struct renderable *renderable1 = item1;
  const struct renderable *renderable2 = item2;

  if(renderable1->camera != renderable2->camera)
    return (uintptr_t)renderable1->camera - (uintptr_t)renderable2->camera;

  if(renderable1->mesh != renderable2->mesh)
    return (uintptr_t)renderable1->mesh - (uintptr_t)renderable2->mesh;

  if(renderable1->texture != renderable2->texture)
    return (uintptr_t)renderable1->texture - (uintptr_t)renderable2->texture;

  return 0;
}

void render_end(void)
{
  // Sort renderable by camera, mesh and texture.
  qsort(renderables.items, renderables.item_count, sizeof *renderables.items, &compar);

  // Draw calls
  {
    struct gl_program program = GL_PROGRAM_LOAD(lib/common/assets/shaders/model);
    glUseProgram(program.id);

    size_t i=0;
    while(i < renderables.item_count)
    {
      size_t j=i+1;
      while(compar(&renderables.items[i], &renderables.items[j]) == 0)
        ++j;

      {
        const struct camera *camera = renderables.items[i].camera;
        const struct mesh *mesh = renderables.items[i].mesh;
        const struct gl_texture_2d *texture = renderables.items[i].texture;

        // View projection matrix.
        fmat4_t VP = fmat4_identity();
        VP = fmat4_mul(camera_view_matrix(camera), VP);
        VP = fmat4_mul(camera_projection_matrix(camera), VP);
        glUniformMatrix4fv(glGetUniformLocation(program.id, "VP"), 1, GL_TRUE, (const float *)&VP);

        // Texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture->id);

        // Mesh
        {
          DYNAMIC_ARRAY_DECLARE(vertices_instanced, struct mesh_vertex_instanced);

          for(size_t k=i; k<j; ++k)
          {
            struct mesh_vertex_instanced vertex_instanced;
            vertex_instanced.transform = transform_matrix(renderables.items[k].transform);
            vertex_instanced.light = renderables.items[k].light;
            DYNAMIC_ARRAY_APPEND(vertices_instanced, vertex_instanced);
          }
          mesh_update_instanced(mesh, vertices_instanced.items, vertices_instanced.item_count);

          glBindVertexArray(mesh->vao);
          glDrawArraysInstanced(GL_TRIANGLES, 0, mesh->count, vertices_instanced.item_count);

          DYNAMIC_ARRAY_CLEAR(vertices_instanced);
        }
      }

      i = j;
    }
  }

  // Clear
  DYNAMIC_ARRAY_CLEAR(renderables);
}

