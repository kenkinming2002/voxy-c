#include <main_game/player_movement.h>
#include <main_game/world.h>

#include <types/world.h>
#include <types/player.h>

#include <core/window.h>

#include <voxy/math/vector.h>
#include <voxy/math/transform.h>

#include <config.h>

void update_player_movement(float dt)
{
  if(!world.player.spawned)
    return;

  if(world.player.inventory.opened)
    return;

  fvec3_t impulse = fvec3_zero();

  if(input_state(KEY_A)) impulse.x -= 1.0f;
  if(input_state(KEY_D)) impulse.x += 1.0f;
  if(input_state(KEY_S)) impulse.y -= 1.0f;
  if(input_state(KEY_W)) impulse.y += 1.0f;

  impulse = transform_local(world.player.base.local_view_transform, impulse);
  impulse.z = 0.0f;
  impulse = fvec3_normalize(impulse);
  impulse = fvec3_mul_scalar(impulse, world.player.base.grounded ? PLAYER_MOVE_SPEED_GROUND : PLAYER_MOVE_SPEED_AIR);
  impulse = fvec3_mul_scalar(impulse, dt);

  entity_apply_impulse(&world.player.base, impulse);

  if(input_state(KEY_SPACE) && world.player.base.grounded)
  {
    world.player.base.grounded = false;
    entity_apply_impulse(&world.player.base, fvec3(0.0f, 0.0f, PLAYER_JUMP_STRENGTH));
  }
}
