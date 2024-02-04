#include "world.h"

#include "ray_cast.h"

static void world_update_player_ray_cast(struct world *world, struct resource_pack *resource_pack)
{
  fvec3_t position  = world->player.transform.translation;
  fvec3_t direction = fvec3_normalize(transform_forward(&world->player.transform));

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
}

void world_update_player_control(struct world *world, struct resource_pack *resource_pack, struct input *input, float dt)
{
  if(!world->player.spawned)
    return;

  static const float MOVE_SPEED = 50.0f;
  static const float PAN_SPEED  = 0.002f;

  fvec3_t rotation    = fvec3_mul_scalar(fvec3(input->mouse_motion.x, input->mouse_motion.y, 0.0f), PAN_SPEED);
  fvec3_t translation = fvec3_mul_scalar(input->keyboard_motion, MOVE_SPEED * dt);

  transform_rotate(&world->player.transform, rotation);
  transform_local_translate(&world->player.transform, translation);

  for(unsigned i=0; i<9; ++i)
    if(input->selects[i])
      world->player.inventory.hotbar_selection = i;

  world->player.inventory.hotbar_selection += input->scroll;
  world->player.inventory.hotbar_selection += 9;
  world->player.inventory.hotbar_selection %= 9;

  struct tile *tile;

  world_update_player_ray_cast(world, resource_pack);
  if(input->state_left && world->player.has_target_destroy && (tile = world_get_tile(world, world->player.target_destroy)))
  {
    int radius = input->state_ctrl ? 2 : 0;
    for(int dz=-radius; dz<=radius; ++dz)
      for(int dy=-radius; dy<=radius; ++dy)
        for(int dx=-radius; dx<=radius; ++dx)
        {
          ivec3_t offset = ivec3(dx, dy, dz);
          if(ivec3_length_squared(offset) <= radius * radius)
            world_tile_set_id(world, ivec3_add(world->player.target_destroy, offset), 0);
        }

    world_update_player_ray_cast(world, resource_pack);
  }

  if(input->state_right && world->player.has_target_place && (tile = world_get_tile(world, world->player.target_place)))
  {
    uint8_t item_id = world->player.inventory.hotbar_items[world->player.inventory.hotbar_selection];
    if(item_id != ITEM_NONE)
    {
      uint8_t block_id = resource_pack->item_infos[item_id].block_id;

      int radius = input->state_ctrl ? 2 : 0;
      for(int dz=-radius; dz<=radius; ++dz)
        for(int dy=-radius; dy<=radius; ++dy)
          for(int dx=-radius; dx<=radius; ++dx)
          {
            ivec3_t offset = ivec3(dx, dy, dz);
            if(ivec3_length_squared(offset) <= radius * radius)
              world_tile_set_id(world, ivec3_add(world->player.target_place, offset), block_id);
          }

      world_update_player_ray_cast(world, resource_pack);
    }
  }
}

