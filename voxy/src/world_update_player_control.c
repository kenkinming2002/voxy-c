#include <types/world.h>
#include <types/block.h>
#include <types/mod.h>

#include <voxy/mod_interface.h>

#include "config.h"
#include <voxy/math/ray_cast.h>
#include <core/window.h>

void world_update_player_control(struct world *world, struct mod *mod, float dt)
{
  if(!world->player.spawned)
    return;

  if(world->player.inventory.opened)
    return;

  if(input_press(KEY_F))
    world->player.third_person = !world->player.third_person;

  //////////////
  /// Camera ///
  //////////////
  fvec3_t rotation = fvec3_mul_scalar(fvec3(mouse_motion.x, -mouse_motion.y, 0.0f), PLAYER_PAN_SPEED);
  world->player.base.local_view_transform = transform_rotate(world->player.base.local_view_transform, rotation);

  ////////////////
  /// Movement ///
  ////////////////
  fvec3_t axis = fvec3_zero();
  if(input_state(KEY_A)) axis.x -= 1.0f;
  if(input_state(KEY_D)) axis.x += 1.0f;
  if(input_state(KEY_S)) axis.y -= 1.0f;
  if(input_state(KEY_W)) axis.y += 1.0f;

  fvec3_t impulse = transform_local(world->player.base.local_view_transform, axis);

  impulse.z = 0.0f;
  impulse = fvec3_normalize(impulse);
  impulse = fvec3_mul_scalar(impulse, world->player.base.grounded ? PLAYER_MOVE_SPEED_GROUND : PLAYER_MOVE_SPEED_AIR);
  impulse = fvec3_mul_scalar(impulse, dt);

  entity_apply_impulse(&world->player.base, impulse);

  ////////////
  /// Jump ///
  ////////////
  if(input_state(KEY_SPACE) && world->player.base.grounded)
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

  if(input_state(BUTTON_LEFT) && world->player.cooldown >= PLAYER_ACTION_COOLDOWN && entity_ray_cast(&world->player.base, world, mod, 20.0f, &position, &normal))
  {
    world->player.cooldown = 0.0f;

    int radius = input_state(KEY_CTRL) ? 2 : 0;
    for(int dz=-radius; dz<=radius; ++dz)
      for(int dy=-radius; dy<=radius; ++dy)
        for(int dx=-radius; dx<=radius; ++dx)
        {
          ivec3_t offset = ivec3(dx, dy, dz);
          if(ivec3_length_squared(offset) <= radius * radius)
            world_block_set_id(world, ivec3_add(position, offset), 0);
        }
  }

  if(input_state(BUTTON_RIGHT) && world->player.cooldown >= PLAYER_ACTION_COOLDOWN && entity_ray_cast(&world->player.base, world, mod, 20.0f, &position, &normal))
  {
    const struct item *item = &world->player.hotbar.items[world->player.hotbar.selection];
    if(item->id != ITEM_NONE)
    {
      world->player.cooldown = 0.0f;

      uint8_t block_id = mod->item_infos[item->id].block_id;

      int radius = input_state(KEY_CTRL) ? 2 : 0;
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

