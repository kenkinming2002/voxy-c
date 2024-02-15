#include <voxy/main_game/player_movement.h>
#include <voxy/main_game/player.h>

#include <voxy/types/player.h>

#include <voxy/core/window.h>

#include <voxy/math/vector.h>
#include <voxy/math/transform.h>

#include <voxy/config.h>

static fvec2_t player_move_direction(void)
{
  fvec2_t direction = fvec2_zero();

  if(input_state(KEY_A)) direction.x -= 1.0f;
  if(input_state(KEY_D)) direction.x += 1.0f;
  if(input_state(KEY_S)) direction.y -= 1.0f;
  if(input_state(KEY_W)) direction.y += 1.0f;

  return direction;
}

void update_player_movement(float dt)
{
  if(!player_spawned)
    return;

  if(player.inventory.opened)
    return;

  entity_move(&player.base, player_move_direction(), player.base.grounded ? PLAYER_MOVE_SPEED_GROUND : PLAYER_MOVE_SPEED_AIR, dt);
  if(input_state(KEY_SPACE))
    entity_jump(&player.base, PLAYER_JUMP_STRENGTH);
}
