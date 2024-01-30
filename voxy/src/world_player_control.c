#include "world.h"

void world_update_player_control(struct world *world, struct window *window, float dt)
{
  static const float MOVE_SPEED = 50.0f;
  static const float PAN_SPEED  = 0.2;

  fvec3_t rotation    = fvec3_zero();
  fvec3_t translation = fvec3_zero();

  window_get_mouse_motion(window, &rotation.yaw, &rotation.pitch);
  window_get_keyboard_motion(window, &translation.x, &translation.y, &translation.z);

  rotation    = fvec3_mul_scalar(rotation, PAN_SPEED * dt);
  translation = fvec3_normalize(translation);
  translation = fvec3_mul_scalar(translation, MOVE_SPEED * dt);

  transform_rotate(&world->player.transform, rotation);
  transform_local_translate(&world->player.transform, translation);
}

