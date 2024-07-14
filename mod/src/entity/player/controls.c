#include "controls.h"

#include <voxy/core/window.h>
#include <voxy/math/direction.h>

void player_entity_update_controls(struct entity *entity, float dt)
{
  struct player_opaque *opaque = entity->opaque;
  if(opaque->inventory_opened)
    return;

  // Third person
  {
    opaque->third_person = opaque->third_person != input_press(KEY_F);
  }

  // Pan
  {
    entity->rotation.yaw   +=  mouse_motion.x * PLAYER_PAN_SPEED;
    entity->rotation.pitch += -mouse_motion.y * PLAYER_PAN_SPEED;
  }

  // Move
  {
    float speed = entity->grounded ? PLAYER_MOVE_SPEED_GROUND : PLAYER_MOVE_SPEED_GROUND;

    fvec2_t axis = fvec2_zero();
    if(input_state(KEY_A)) { axis.x -= 1.0f; }
    if(input_state(KEY_D)) { axis.x += 1.0f; }
    if(input_state(KEY_S)) { axis.y -= 1.0f; }
    if(input_state(KEY_W)) { axis.y += 1.0f; }

    fvec3_t impulse;
    if(fvec2_length_squared(axis) != 0.0f)
    {
      impulse = fvec3(axis.x, axis.y, 0.0f);
      impulse = entity_local_to_global(entity, impulse);
      impulse = fvec3(impulse.x, impulse.y, 0.0f);
      impulse = fvec3_normalize(impulse);
      impulse = fvec3_mul_scalar(impulse, speed * dt);
    }
    else
    {
      impulse = fvec3(entity->velocity.x, entity->velocity.y, 0.0f);
      impulse = fvec3_neg(impulse);
      if(fvec3_length(impulse) > speed * dt)
      {
        impulse = fvec3_normalize(impulse);
        impulse = fvec3_mul_scalar(impulse, speed * dt);
      }
    }
    entity_apply_impulse(entity, impulse);
  }

  // Jump
  {
    if(entity->grounded && input_press(KEY_SPACE))
    {
      fvec3_t impulse;
      impulse = direction_as_fvec(DIRECTION_TOP);
      impulse = fvec3_mul_scalar(impulse, PLAYER_JUMP_STRENGTH);
      entity_apply_impulse(entity, impulse);
    }
  }
}
