#include "world.h"

#include "ray_cast.h"

void world_update_player_control(struct world *world, struct resource_pack *resource_pack, struct input *input, float dt)
{
  static const float MOVE_SPEED = 50.0f;
  static const float PAN_SPEED  = 0.002f;

  fvec3_t rotation    = fvec3_mul_scalar(fvec3(input->mouse_motion.x, input->mouse_motion.y, 0.0f), PAN_SPEED);
  fvec3_t translation = fvec3_mul_scalar(input->keyboard_motion, MOVE_SPEED * dt);

  transform_rotate(&world->player.transform, rotation);
  transform_local_translate(&world->player.transform, translation);

  for(unsigned i=1; i<=9; ++i)
    if(input->selects[i-1])
      world->player.selection = i;

  world->player.selection += input->scroll;
  world->player.selection -= 1;
  world->player.selection += 9;
  world->player.selection %= 9;
  world->player.selection += 1;

  fvec3_t position  = world->player.transform.translation;
  fvec3_t direction = fvec3_normalize(transform_forward(&world->player.transform));

retry:
  world->player.has_target_destroy = false;
  world->player.has_target_place   = false;

  struct ray_cast ray_cast;
  ray_cast_init(&ray_cast, position);
  while(ray_cast.distance < 20.0f)
  {
    struct tile *tile = world_get_tile(world, ray_cast.iposition);
    if(tile && resource_pack->block_infos[tile->id].type == BLOCK_TYPE_OPAQUE)
    {
      world->player.has_target_destroy = true;
      world->player.target_destroy     = ray_cast.iposition;
      break;
    }
    world->player.has_target_place = true;
    world->player.target_place     = ray_cast.iposition;
    ray_cast_step(&ray_cast, direction);
  }

  struct tile *tile;

  if(input->state_left && world->player.has_target_destroy && (tile = world_get_tile(world, world->player.target_destroy)))
  {
    tile->id = 0;
    input->state_left = false;

    world_invalidate_tile(world, world->player.target_destroy);
    world_invalidate_tile(world, ivec3_add(world->player.target_destroy, ivec3(-1, 0, 0)));
    world_invalidate_tile(world, ivec3_add(world->player.target_destroy, ivec3( 1, 0, 0)));
    world_invalidate_tile(world, ivec3_add(world->player.target_destroy, ivec3(0, -1, 0)));
    world_invalidate_tile(world, ivec3_add(world->player.target_destroy, ivec3(0,  1, 0)));
    world_invalidate_tile(world, ivec3_add(world->player.target_destroy, ivec3(0, 0, -1)));
    world_invalidate_tile(world, ivec3_add(world->player.target_destroy, ivec3(0, 0,  1)));

    goto retry;
  }

  if(input->state_right && world->player.has_target_place && (tile = world_get_tile(world, world->player.target_place)))
  {
    tile->id = 2;
    input->state_right = false;

    world_invalidate_tile(world, world->player.target_destroy);
    world_invalidate_tile(world, ivec3_add(world->player.target_destroy, ivec3(-1, 0, 0)));
    world_invalidate_tile(world, ivec3_add(world->player.target_destroy, ivec3( 1, 0, 0)));
    world_invalidate_tile(world, ivec3_add(world->player.target_destroy, ivec3(0, -1, 0)));
    world_invalidate_tile(world, ivec3_add(world->player.target_destroy, ivec3(0,  1, 0)));
    world_invalidate_tile(world, ivec3_add(world->player.target_destroy, ivec3(0, 0, -1)));
    world_invalidate_tile(world, ivec3_add(world->player.target_destroy, ivec3(0, 0,  1)));

    goto retry;
  }
}

