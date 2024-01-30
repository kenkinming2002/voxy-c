#include "world.h"

void world_update_player_control(struct world *world, struct input *input, float dt)
{
  static const float MOVE_SPEED = 50.0f;
  static const float PAN_SPEED  = 0.2f;

  fvec3_t rotation    = fvec3_mul_scalar(fvec3(input->mouse_motion.x, input->mouse_motion.y, 0.0f), PAN_SPEED * dt);
  fvec3_t translation = fvec3_mul_scalar(input->keyboard_motion, MOVE_SPEED * dt);

  transform_rotate(&world->player.transform, rotation);
  transform_local_translate(&world->player.transform, translation);
}

