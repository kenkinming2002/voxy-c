#include <voxy/scene/main_game/update/physics/physics.h>
#include <voxy/scene/main_game/update/physics/swept.h>

#include <voxy/scene/main_game/types/block.h>
#include <voxy/scene/main_game/types/entity.h>

#include <voxy/scene/main_game/states/chunks.h>

#include <voxy/scene/main_game/config.h>
#include <voxy/scene/main_game/mod.h>

#include <libcommon/math/aabb.h>
#include <libcommon/math/direction.h>
#include <libcommon/core/log.h>
#include <libcommon/utils/utils.h>

#include <stdbool.h>

#define FALL_DAMAGE_TOLERANCE 2.0f
#define FALL_DAMAGE_FACTOR 0.5f

static void entity_physics_apply_law(struct entity *entity, float dt)
{
  entity->velocity = fvec3_add(entity->velocity, fvec3(0.0f, 0.0f, -PHYSICS_GRAVITY * dt));   // Gravity
  entity->velocity = fvec3_mul_scalar(entity->velocity, expf(-dt * (entity->grounded ? PHYSICS_DRAG_GROUND : PHYSICS_DRAG_AIR))); // Drag
}

static void entity_physics_resolve_collision(struct entity *entity, float dt)
{
  for(;;)
  {
    bool hit = false;
    float min_t;
    float max_s;
    direction_t min_direction;

    const fvec3_t offset = fvec3_mul_scalar(entity->velocity, dt);

    const struct entity_info *entity_info = query_entity_info(entity->id);
    const aabb3_t entity_hitbox = aabb3(fvec3_add(entity->position, entity_info->hitbox_offset), entity_info->hitbox_dimension);
    const aabb3_t entity_hitbox_expanded = aabb3_expand(entity_hitbox, offset);

    const ivec3_t block_position_min = fvec3_as_ivec3_round(aabb3_min_corner(entity_hitbox_expanded));
    const ivec3_t block_position_max = fvec3_as_ivec3_round(aabb3_max_corner(entity_hitbox_expanded));
    for(int z=block_position_min.z; z<=block_position_max.z; ++z)
      for(int y=block_position_min.y; y<=block_position_max.y; ++y)
        for(int x=block_position_min.x; x<=block_position_max.x; ++x)
        {
          const ivec3_t block_position = ivec3(x, y, z);
          const block_id_t block_id = world_get_block_id(block_position);
          if(block_id != BLOCK_NONE && query_block_info(block_id)->physics_type == BLOCK_PHYSICS_TYPE_CUBE)
          {
            const aabb3_t block_hitbox = aabb3(ivec3_as_fvec3(block_position), fvec3(1.0f, 1.0f, 1.0f));

            float t1;
            float t2;

            float s1;
            float s2;

            direction_t direction1;
            direction_t direction2;

            if(swept_aabb3(entity_hitbox, block_hitbox, offset, &t1, &t2, &s1, &s2, &direction1, &direction2))
              if((0.0f <= t1 && t1 < 1.0f) && (!hit || min_t > t1 || (min_t >= t1 && max_s < s1)))
              {
                hit = true;
                min_t = t1;
                max_s = s1;
                min_direction = direction1;
              }
          }
        }

    // Update entity velocity accordingly if we hit something. We have to make
    // sure that we are actually updating something and not get stuck in an
    // infinite loop.
    if(hit && entity->velocity.values[direction_axis(min_direction)] != 0.0f)
      entity->velocity.values[direction_axis(min_direction)] *= min_t;
    else
      break;
  }
}

static void entity_physics_integrate(struct entity *entity, float dt)
{
  entity->position = fvec3_add(entity->position, fvec3_mul_scalar(entity->velocity, dt));
}

static bool entity_is_grounded(struct entity *entity)
{
  const struct entity_info *entity_info = query_entity_info(entity->id);
  aabb3_t entity_hitbox = aabb3(fvec3_add(entity->position, entity_info->hitbox_offset), entity_info->hitbox_dimension);
  entity_hitbox.dimension.x *= 0.999f;
  entity_hitbox.dimension.y *= 0.999f;
  entity_hitbox.center.z -= 0.001f;

  const ivec3_t block_position_min = fvec3_as_ivec3_round(aabb3_min_corner(entity_hitbox));
  const ivec3_t block_position_max = fvec3_as_ivec3_round(aabb3_max_corner(entity_hitbox));
  for(int z=block_position_min.z; z<=block_position_max.z; ++z)
    for(int y=block_position_min.y; y<=block_position_max.y; ++y)
      for(int x=block_position_min.x; x<=block_position_max.x; ++x)
      {
        const ivec3_t block_position = ivec3(x, y, z);
        const block_id_t block_id = world_get_block_id(block_position);
        if(block_id != BLOCK_NONE && query_block_info(block_id)->physics_type == BLOCK_PHYSICS_TYPE_CUBE)
          return true;
      }

  return false;
}

static void entity_physics_update_grounded(struct entity *entity)
{
  const bool grounded = entity_is_grounded(entity);
  if(!entity->grounded && grounded)
  {
    const float fall_distance = MAX(entity->max_height - entity->position.z - FALL_DAMAGE_TOLERANCE, 0.0f);
    entity->health -= fall_distance * FALL_DAMAGE_FACTOR;
    entity->max_height = entity->position.z;
  }
  entity->grounded = grounded;
}

static void entity_update_physics(struct entity *entity, float dt)
{
  if(entity->health <= 0.0f)
    return;

  entity->max_height = MAX(entity->max_height, entity->position.z);
  entity_physics_apply_law(entity, dt);
  entity_physics_resolve_collision(entity, dt);
  entity_physics_integrate(entity, dt);
  entity_physics_update_grounded(entity);
}

void update_physics(float dt)
{
  world_for_each_chunk(chunk)
  {
    for(size_t i=0; i<chunk->entities.item_count; ++i)
      entity_update_physics(&chunk->entities.items[i], dt);

    if(chunk->entities.item_count != 0)
      chunk_invalidate_data(chunk);
  }
}
