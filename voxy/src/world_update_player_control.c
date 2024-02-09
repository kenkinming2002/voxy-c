#include <types/world.h>
#include <types/block.h>

#include "config.h"
#include "ray_cast.h"
#include "resource_pack.h"
#include "window.h"

void world_update_player_control(struct world *world, struct resource_pack *resource_pack, struct window *window, float dt)
{
  if(!world->player.spawned)
    return;

  if(world->player.inventory.opened)
    return;

  if(window->presses & (1ULL << KEY_F))
    world->player.third_person = !world->player.third_person;

  //////////////
  /// Camera ///
  //////////////
  fvec3_t rotation = fvec3_mul_scalar(fvec3(window->mouse_motion.x, -window->mouse_motion.y, 0.0f), PLAYER_PAN_SPEED);
  transform_rotate(&world->player.base.local_view_transform, rotation);

  ////////////////
  /// Movement ///
  ////////////////
  fvec3_t axis = fvec3_zero();
  if(window->states & (1ULL << KEY_A)) axis.x -= 1.0f;
  if(window->states & (1ULL << KEY_D)) axis.x += 1.0f;
  if(window->states & (1ULL << KEY_S)) axis.y -= 1.0f;
  if(window->states & (1ULL << KEY_W)) axis.y += 1.0f;

  fvec3_t impulse = transform_local(&world->player.base.local_view_transform, axis);

  impulse.z = 0.0f;
  impulse = fvec3_normalize(impulse);
  impulse = fvec3_mul_scalar(impulse, world->player.base.grounded ? PLAYER_MOVE_SPEED_GROUND : PLAYER_MOVE_SPEED_AIR);
  impulse = fvec3_mul_scalar(impulse, dt);

  entity_apply_impulse(&world->player.base, impulse);

  ////////////
  /// Jump ///
  ////////////
  if(window->states & (1ULL << KEY_SPACE) && world->player.base.grounded)
  {
    world->player.base.grounded = false;
    entity_apply_impulse(&world->player.base, fvec3(0.0f, 0.0f, PLAYER_JUMP_STRENGTH));
  }

  ///////////////////////////////////
  /// Block placement/destruction ///
  ///////////////////////////////////
  world->player.cooldown += dt;

  ivec3_t position;
  ivec3_t normal;

  if((window->states & 1ULL << BUTTON_LEFT) && world->player.cooldown >= PLAYER_ACTION_COOLDOWN && entity_ray_cast(&world->player.base, world, resource_pack, 20.0f, &position, &normal))
  {
    world->player.cooldown = 0.0f;

    int radius = window->states & 1ULL << KEY_CTRL ? 2 : 0;
    for(int dz=-radius; dz<=radius; ++dz)
      for(int dy=-radius; dy<=radius; ++dy)
        for(int dx=-radius; dx<=radius; ++dx)
        {
          ivec3_t offset = ivec3(dx, dy, dz);
          if(ivec3_length_squared(offset) <= radius * radius)
            world_block_set_id(world, ivec3_add(position, offset), 0);
        }
  }

  if((window->states & 1ULL << BUTTON_RIGHT) && world->player.cooldown >= PLAYER_ACTION_COOLDOWN && entity_ray_cast(&world->player.base, world, resource_pack, 20.0f, &position, &normal))
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
              world_block_set_id(world, ivec3_add(ivec3_add(position, normal), offset), block_id);
          }
    }
  }
}

