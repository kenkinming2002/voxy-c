#include <voxy/main_game/player_camera.h>
#include <voxy/main_game/world.h>

#include <voxy/types/world.h>
#include <voxy/types/player.h>

#include <voxy/core/window.h>

#include <voxy/config.h>

void update_player_camera(void)
{
  if(!world.player.spawned)
    return;

  if(world.player.inventory.opened)
    return;

  fvec3_t rotation = fvec3_mul_scalar(fvec3(mouse_motion.x, -mouse_motion.y, 0.0f), PLAYER_PAN_SPEED);
  world.player.base.local_view_transform = transform_rotate(world.player.base.local_view_transform, rotation);
  world.player.third_person = world.player.third_person != input_press(KEY_F); // Bitwise: ^, Logical: !=
}
