#include "physics.h"
#include "swept.h"

#include <libcommon/math/aabb.h>
#include <libcommon/math/direction.h>
#include <libcommon/core/log.h>
#include <libcommon/core/profile.h>
#include <libcommon/utils/utils.h>

#include <stdbool.h>

#define GRAVITY 9.8f

#define GRAVITY 9.8f
#define DRAG_GROUND 2.0f
#define DRAG_AIR 0.5f

#define FALL_DAMAGE_TOLERANCE 2.0f
#define FALL_DAMAGE_FACTOR 0.5f

static void entity_physics_apply_law(struct voxy_entity *entity, float dt)
{
  entity->velocity = fvec3_add(entity->velocity, fvec3(0.0f, 0.0f , -GRAVITY * dt));
  entity->velocity = fvec3_mul_scalar(entity->velocity, expf(-dt * (entity->grounded ? DRAG_GROUND : DRAG_AIR)));
}

static int compar(const void *ptr1, const void *ptr2)
{
  const struct contact3 *contact1 = ptr1;
  const struct contact3 *contact2 = ptr2;
  return -contact3_compare(*contact1, *contact2);
}

static bool entity_physics_resolve_collision_once(
    struct voxy_block_registry *block_registry,
    struct voxy_entity_registry *entity_registry,
    struct voxy_chunk_manager *chunk_manager,
    struct voxy_entity *entity,
    float dt)
{
  // Figure out all the contacts and sort them.
  DYNAMIC_ARRAY_DECLARE(contacts, struct contact3);
  {
    const fvec3_t offset = fvec3_mul_scalar(entity->velocity, dt);

    const struct voxy_entity_info entity_info = voxy_entity_registry_query_entity(entity_registry, entity->id);
    const aabb3_t entity_hitbox = aabb3(fvec3_add(entity->position, entity_info.hitbox_offset), entity_info.hitbox_dimension);
    const aabb3_t entity_hitbox_expanded = aabb3_expand(entity_hitbox, offset);

    const ivec3_t block_position_min = ivec3_sub_scalar(fvec3_as_ivec3_round(aabb3_min_corner(entity_hitbox_expanded)), 1);
    const ivec3_t block_position_max = ivec3_add_scalar(fvec3_as_ivec3_round(aabb3_max_corner(entity_hitbox_expanded)), 1);
    for(int z=block_position_min.z; z<=block_position_max.z; ++z)
      for(int y=block_position_min.y; y<=block_position_max.y; ++y)
        for(int x=block_position_min.x; x<=block_position_max.x; ++x)
        {
          const ivec3_t block_position = ivec3(x, y, z);
          const uint8_t block_id = voxy_chunk_manager_get_block_id(chunk_manager, block_position, UINT8_MAX);
          if(block_id != UINT8_MAX && voxy_block_registry_query_block(block_registry, block_id).collide)
          {
            const aabb3_t block_hitbox = aabb3(ivec3_as_fvec3(block_position), fvec3(1.0f, 1.0f, 1.0f));

            struct contact3 contact1;
            struct contact3 contact2;
            if(swept_aabb3(entity_hitbox, block_hitbox, offset, &contact1, &contact2))
              if(0.0f <= contact1.time && contact1.time < 1.0f)
                DYNAMIC_ARRAY_APPEND(contacts, contact1);
          }
        }

    qsort(contacts.items, contacts.item_count, sizeof *contacts.items, compar);
  }

  // Resolve collision based on contacts.
  bool resolved = false;
  for(size_t i=0; i<contacts.item_count; ++i)
  {
    const float nice = fvec3_dot(entity->velocity, contacts.items[i].normal);
    if(nice < 0.0f)
    {
      // We need to increase the strength of our impulse a bit by multiplying it
      // with 1.0001f. This so that we do not get accidentally stuck in a block
      // due to floating point inaccurancy. Technically speaking, this small
      // adjustment can be noticeable if our speed is high.
      //
      // FIXME: This is still incorrect.
      const float strength = -nice * (1.0f - contacts.items[i].time) * 1.0001f;
      const fvec3_t impulse = fvec3_mul_scalar(contacts.items[i].normal, strength);
      entity->velocity = fvec3_add(entity->velocity, impulse);

      resolved = true;
      break;
    }
  }

  DYNAMIC_ARRAY_CLEAR(contacts);
  return resolved;
}

static void entity_physics_resolve_collision(
    struct voxy_block_registry *block_registry,
    struct voxy_entity_registry *entity_registry,
    struct voxy_chunk_manager *chunk_manager,
    struct voxy_entity *entity,
    float dt)
{
  while(entity_physics_resolve_collision_once(block_registry, entity_registry, chunk_manager, entity, dt));
}

static void entity_physics_integrate(struct voxy_entity *entity, float dt)
{
  entity->position = fvec3_add(entity->position, fvec3_mul_scalar(entity->velocity, dt));
}

static bool entity_is_grounded(
    struct voxy_block_registry *block_registry,
    struct voxy_entity_registry *entity_registry,
    struct voxy_chunk_manager *chunk_manager,
    struct voxy_entity *entity)
{
  const struct voxy_entity_info entity_info = voxy_entity_registry_query_entity(entity_registry, entity->id);
  aabb3_t entity_hitbox = aabb3(fvec3_add(entity->position, entity_info.hitbox_offset), entity_info.hitbox_dimension);
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
        const uint8_t block_id = voxy_chunk_manager_get_block_id(chunk_manager, block_position, UINT8_MAX);
        if(block_id != UINT8_MAX && voxy_block_registry_query_block(block_registry, block_id).collide)
          return true;
      }

  return false;
}

static void entity_physics_update_grounded(
    struct voxy_block_registry *block_registry,
    struct voxy_entity_registry *entity_registry,
    struct voxy_chunk_manager *chunk_manager,
    struct voxy_entity *entity)
{
  entity->grounded = entity_is_grounded(block_registry, entity_registry, chunk_manager, entity);
}

static void entity_update_physics(
    struct voxy_block_registry *block_registry,
    struct voxy_entity_registry *entity_registry,
    struct voxy_chunk_manager *chunk_manager,
    struct voxy_entity *entity,
    float dt)
{
  entity_physics_apply_law(entity, dt);
  entity_physics_resolve_collision(block_registry, entity_registry, chunk_manager, entity, dt);
  entity_physics_integrate(entity, dt);
  entity_physics_update_grounded(block_registry, entity_registry, chunk_manager, entity);
}

void physics_update(
    struct voxy_block_registry *block_registry,
    struct voxy_entity_registry *entity_registry,
    struct voxy_chunk_manager *chunk_manager,
    struct voxy_entity_manager *entity_manager,
    float dt)
{
  profile_begin();

  for(entity_handle_t handle=0; handle<entity_manager->allocator.entities.item_count; ++handle)
  {
    struct voxy_entity *entity = &entity_manager->allocator.entities.items[handle];
    if(!entity->alive)
      continue;

    entity_update_physics(block_registry, entity_registry, chunk_manager, entity, dt);
  }

  profile_end();
}
