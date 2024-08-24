#include "debug_gizmos.h"

#include <voxy/scene/main_game/states/camera.h>
#include <voxy/scene/main_game/states/chunks.h>

#include <libcommon/math/matrix.h>
#include <libcommon/graphics/camera.h>
#include <libcommon/graphics/gl.h>

void main_game_render_debug_gizmos()
{
    fmat4_t VP = fmat4_identity();
    VP = fmat4_mul(camera_view_matrix(&world_camera),       VP);
    VP = fmat4_mul(camera_projection_matrix(&world_camera), VP);

    struct gl_program program = GL_PROGRAM_LOAD(bin/voxy/assets/shaders/outline);
    glUseProgram(program.id);
    glUniformMatrix4fv(glGetUniformLocation(program.id, "VP"), 1, GL_TRUE, (const float *)&VP);

    world_for_each_chunk(chunk)
      for(size_t i=0; i<chunk->entities.item_count; ++i)
      {
        struct entity *entity = &chunk->entities.items[i];
        const struct entity_info *entity_info = query_entity_info(entity->id);

        fvec3_t hitbox_position = fvec3_add(entity->position, entity_info->hitbox_offset);
        fvec3_t hitbox_dimension = entity_info->hitbox_dimension;

        glUniform3f(glGetUniformLocation(program.id, "position"),  hitbox_position.x, hitbox_position.y, hitbox_position.z);
        glUniform3f(glGetUniformLocation(program.id, "dimension"),  hitbox_dimension.x, hitbox_dimension.y, hitbox_dimension.z);
        glUniform4f(glGetUniformLocation(program.id, "color"),     1.0f, 0.0f, 0.0f, 1.0f);
        glDrawArrays(GL_LINES, 0, 24);
      }
}
