#include <voxy/scene/main_game/render/entities.h>
#include <voxy/scene/main_game/render/debug.h>

#include <voxy/scene/main_game/states/camera.h>
#include <voxy/scene/main_game/states/chunks.h>

#include <voxy/graphics/camera.h>
#include <voxy/graphics/mesh.h>
#include <voxy/graphics/gl.h>

#include <string.h>

static struct gl_texture_2d entity_texture(void)
{
  static int initialized = 0;
  static struct gl_texture_2d texture;
  if(!initialized)
  {
    if(gl_texture_2d_load(&texture, "assets/models/pig.png") != 0)
    {
      LOG_ERROR("Failed to load entity texture");
      exit(EXIT_FAILURE);
    }
    initialized = true;
  }
  return texture;
}

static struct mesh entity_mesh(void)
{
  static int initialized = 0;
  static struct mesh mesh;
  if(!initialized)
  {
    if(mesh_load(&mesh, "assets/models/pig.obj") != 0)
    {
      LOG_ERROR("Failed to load entity mesh");
      exit(EXIT_FAILURE);
    }
    initialized = true;
  }
  return mesh;
}

void main_game_render_entities(void)
{
  // TODO: Actual entity rendering
  if(main_game_render_get_debug())
  {
    fmat4_t VP = fmat4_identity();
    VP = fmat4_mul(camera_view_matrix(&world_camera),       VP);
    VP = fmat4_mul(camera_projection_matrix(&world_camera), VP);

    struct gl_program program = GL_PROGRAM_LOAD(outline);
    glUseProgram(program.id);
    glUniformMatrix4fv(glGetUniformLocation(program.id, "VP"), 1, GL_TRUE, (const float *)&VP);

    world_for_each_chunk(chunk)
      if(chunk->data)
        for(size_t i=0; i<chunk->data->entity_count; ++i)
        {
          struct entity *entity = &chunk->data->entities[i];
          const struct entity_info *entity_info = query_entity_info(entity->id);

          // FIXME: Skip rendering of current active player.

          fvec3_t hitbox_position = fvec3_add(entity->position, entity_info->hitbox_offset);
          fvec3_t hitbox_dimension = entity_info->hitbox_dimension;

          glUniform3f(glGetUniformLocation(program.id, "position"),  hitbox_position.x, hitbox_position.y, hitbox_position.z);
          glUniform3f(glGetUniformLocation(program.id, "dimension"),  hitbox_dimension.x, hitbox_dimension.y, hitbox_dimension.z);
          glUniform4f(glGetUniformLocation(program.id, "color"),     1.0f, 0.0f, 0.0f, 1.0f);
          glDrawArrays(GL_LINES, 0, 24);
        }
  }

  {
    struct gl_program program = GL_PROGRAM_LOAD(mesh);
    glUseProgram(program.id);

    struct mesh mesh = entity_mesh();
    struct gl_texture_2d texture = entity_texture();

    world_for_each_chunk(chunk)
      if(chunk->data)
        for(size_t i=0; i<chunk->data->entity_count; ++i)
        {
          struct entity *entity = &chunk->data->entities[i];
          const struct entity_info *entity_info = query_entity_info(entity->id);

          // A hack
          if(strcmp(entity_info->name, "player") == 0)
            continue;

          transform_t transform = entity_transform(entity);
          transform.rotation.pitch = 0.0;

          fmat4_t MVP = fmat4_identity();
          MVP = fmat4_mul(transform_matrix(transform), MVP);
          MVP = fmat4_mul(camera_view_matrix(&world_camera), MVP);
          MVP = fmat4_mul(camera_projection_matrix(&world_camera), MVP);
          glUniformMatrix4fv(glGetUniformLocation(program.id, "MVP"), 1, GL_TRUE, (const float *)&MVP);

          glActiveTexture(GL_TEXTURE0);
          glBindTexture(GL_TEXTURE_2D, texture.id);

          glBindVertexArray(mesh.vao);
          glDrawArrays(GL_TRIANGLES, 0, mesh.count);
        }
  }
}
