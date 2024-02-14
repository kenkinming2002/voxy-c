#include <voxy/main_game/player_camera.h>
#include <voxy/main_game/player.h>

#include <voxy/types/player.h>

#include <voxy/core/window.h>

#include <voxy/config.h>

void update_player_camera(void)
{
  if(!player_spawned)
    return;

  if(player.inventory.opened)
    return;

  fvec3_t rotation = fvec3_mul_scalar(fvec3(mouse_motion.x, -mouse_motion.y, 0.0f), PLAYER_PAN_SPEED);
  player.base.local_view_transform = transform_rotate(player.base.local_view_transform, rotation);
  player.third_person = player.third_person != input_press(KEY_F); // Bitwise: ^, Logical: !=
}
