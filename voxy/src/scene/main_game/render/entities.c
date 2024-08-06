#include <voxy/scene/main_game/render/entities.h>
#include <voxy/scene/main_game/render/debug.h>

#include <voxy/scene/main_game/states/camera.h>
#include <voxy/scene/main_game/states/chunks.h>
#include <voxy/scene/main_game/render/assets.h>

#include <libcommon/graphics/camera.h>
#include <libcommon/graphics/mesh.h>
#include <libcommon/graphics/gl.h>

void main_game_render_entities(void)
{
  // TODO: Actual entity rendering
  if(main_game_render_get_debug())
  {
    fmat4_t VP = fmat4_identity();
    VP = fmat4_mul(camera_view_matrix(&world_camera),       VP);
    VP = fmat4_mul(camera_projection_matrix(&world_camera), VP);

    struct gl_program program = GL_PROGRAM_LOAD(voxy/assets/shaders/outline);
    glUseProgram(program.id);
    glUniformMatrix4fv(glGetUniformLocation(program.id, "VP"), 1, GL_TRUE, (const float *)&VP);

    world_for_each_chunk(chunk)
      for(size_t i=0; i<chunk->entities.item_count; ++i)
      {
        struct entity *entity = &chunk->entities.items[i];
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

  world_for_each_chunk(chunk)
    for(size_t i=0; i<chunk->entities.item_count; ++i)
    {
      const struct entity *entity = &chunk->entities.items[i];
      const struct entity_info *entity_info = query_entity_info(entity->id);
      entity_info->on_render(entity, &world_camera);
    }
}
