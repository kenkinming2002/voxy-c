#include "world.h"

void world_update_player_control(struct world *world, struct window *window, float dt)
{
  static const float MOVE_SPEED = 50.0f;
  static const float PAN_SPEED  = 0.2;

  struct vec3 rotation = vec3_zero();
  struct vec3 translation = vec3_zero();

  window_get_mouse_motion(window, &rotation.yaw, &rotation.pitch);
  window_get_keyboard_motion(window, &translation.x, &translation.y, &translation.z);

  rotation    = vec3_mul_s(rotation, PAN_SPEED * dt);
  translation = vec3_normalize(translation);
  translation = vec3_mul_s(translation, MOVE_SPEED * dt);

  transform_rotate(&world->player_transform, rotation);
  transform_local_translate(&world->player_transform, translation);
}

