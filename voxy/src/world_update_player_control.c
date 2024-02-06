#include "world.h"

#include "ray_cast.h"
#include "resource_pack.h"
#include "window.h"

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

void world_update_player_control(struct world *world, struct resource_pack *resource_pack, struct window *window, float dt)
{
  if(!world->player.spawned)
    return;

  if(world->player.inventory.opened)
    return;

  if(window->presses & (1ULL << KEY_F))
    world->player.third_person = !world->player.third_person;

  ////////////////
  /// Movement ///
  ////////////////
  fvec3_t rotation = fvec3_mul_scalar(fvec3(window->mouse_motion.x, -window->mouse_motion.y, 0.0f), PLAYER_PAN_SPEED);
  fvec3_t translation = fvec3_zero();

  if(window->states & (1ULL << KEY_A))     translation.x -= 1.0f;
  if(window->states & (1ULL << KEY_D))     translation.x += 1.0f;
  if(window->states & (1ULL << KEY_S))     translation.y -= 1.0f;
  if(window->states & (1ULL << KEY_W))     translation.y += 1.0f;
  if(window->states & (1ULL << KEY_SHIFT)) translation.z -= 1.0f;
  if(window->states & (1ULL << KEY_SPACE)) translation.z += 1.0f;

  translation = fvec3_normalize(translation);
  translation = fvec3_mul_scalar(translation, PLAYER_MOVE_SPEED * dt);

  transform_rotate(&world->player.transform, rotation);
  transform_local_translate(&world->player.transform, translation);

  ///////////////////////////////////
  /// Block placement/destruction ///
  ///////////////////////////////////
  world->player.cooldown += dt;

  struct tile *tile;

  world_update_player_ray_cast(world, resource_pack);
  if(world->player.cooldown >= PLAYER_ACTION_COOLDOWN && (window->states & 1ULL << BUTTON_LEFT) && world->player.has_target_destroy && (tile = world_get_tile(world, world->player.target_destroy)))
  {
    world->player.cooldown = 0.0f;

    int radius = window->states & 1ULL << KEY_CTRL ? 2 : 0;
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

  if(world->player.cooldown >= PLAYER_ACTION_COOLDOWN && (window->states & 1ULL << BUTTON_RIGHT) && world->player.has_target_place && (tile = world_get_tile(world, world->player.target_place)))
  {
    const struct item *item = &world->player.hotbar.items[world->player.hotbar.selection];
    if(item->id != ITEM_NONE)
    {
      world->player.cooldown = 0.0f;

      uint8_t block_id = resource_pack->item_infos[item->id].block_id;

      int radius = window->states & 1ULL << KEY_CTRL ? 2 : 0;
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

