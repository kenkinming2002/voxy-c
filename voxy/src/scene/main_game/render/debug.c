#include <voxy/scene/main_game/render/debug.h>

#include <voxy/scene/main_game/states/camera.h>
#include <voxy/scene/main_game/states/chunks.h>

#include <voxy/graphics/camera.h>
#include <voxy/graphics/gl.h>

static bool g_debug;

void main_game_render_set_debug(bool debug)
{
  g_debug = debug;
}

bool main_game_render_get_debug(void)
{
  return g_debug;
}

void main_game_render_debug(void)
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

    // Render chunks outline
    world_for_each_chunk(chunk)
      if(chunk->render_info)
        if(!chunk->render_info->culled)
        {
          fvec3_t chunk_position = fvec3_add_scalar(ivec3_as_fvec3(ivec3_mul_scalar(chunk->position, CHUNK_WIDTH)), (CHUNK_WIDTH-1) * 0.5f);
          fvec3_t chunk_dimension = fvec3(CHUNK_WIDTH, CHUNK_WIDTH, CHUNK_WIDTH);

          glUniform3f(glGetUniformLocation(program.id, "position"), chunk_position.x, chunk_position.y, chunk_position.z);
          glUniform3f(glGetUniformLocation(program.id, "dimension"), chunk_dimension.x, chunk_dimension.y, chunk_dimension.z);
          glUniform4f(glGetUniformLocation(program.id, "color"), 1.0f, 1.0f, 0.0f, 1.0f);
          glDrawArrays(GL_LINES, 0, 24);
        }

    // Render entities outline
    world_for_each_chunk(chunk)
      if(chunk->data)
        for(size_t i=0; i<chunk->data->entities.item_count; ++i)
        {
          struct entity *entity = &chunk->data->entities.items[i];
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
}
