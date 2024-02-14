#include <voxy/main_game/player_movement.h>
#include <voxy/main_game/world.h>
#include <voxy/main_game/player.h>

#include <voxy/types/world.h>
#include <voxy/types/player.h>

#include <voxy/core/window.h>

#include <voxy/math/vector.h>
#include <voxy/math/transform.h>

#include <voxy/config.h>

void update_player_movement(float dt)
{
  if(!player_spawned)
    return;

  if(player.inventory.opened)
    return;

  fvec3_t impulse = fvec3_zero();

  if(input_state(KEY_A)) impulse.x -= 1.0f;
  if(input_state(KEY_D)) impulse.x += 1.0f;
  if(input_state(KEY_S)) impulse.y -= 1.0f;
  if(input_state(KEY_W)) impulse.y += 1.0f;

  impulse = transform_local(player.base.local_view_transform, impulse);
  impulse.z = 0.0f;
  impulse = fvec3_normalize(impulse);
  impulse = fvec3_mul_scalar(impulse, player.base.grounded ? PLAYER_MOVE_SPEED_GROUND : PLAYER_MOVE_SPEED_AIR);
  impulse = fvec3_mul_scalar(impulse, dt);

  entity_apply_impulse(&player.base, impulse);

  if(input_state(KEY_SPACE) && player.base.grounded)
  {
    player.base.grounded = false;
    entity_apply_impulse(&player.base, fvec3(0.0f, 0.0f, PLAYER_JUMP_STRENGTH));
  }
}
