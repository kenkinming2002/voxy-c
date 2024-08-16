#include <voxy/scene/main_game/types/entity.h>

#include <voxy/scene/main_game/types/block.h>
#include <voxy/scene/main_game/types/registry.h>

#include <voxy/scene/main_game/states/chunks.h>

#include <voxy/scene/main_game/mod.h>

#include <libcommon/math/transform.h>
#include <libcommon/math/ray_cast.h>
#include <libcommon/core/log.h>

void entity_init(struct entity *entity, fvec3_t position, fvec3_t rotation, fvec3_t velocity, float max_health, float health)
{
  entity->position = position;
  entity->rotation = rotation;
  entity->velocity = velocity;

  entity->max_health = max_health;
  entity->health = health;

  entity->grounded = false;
  entity->max_height = entity->position.z;

  entity->remove = false;
}

void entity_fini(struct entity *entity)
{
  const struct entity_info *info = query_entity_info(entity->id);
  if(info->on_dispose)
    info->on_dispose(entity);
}

transform_t entity_transform(const struct entity *entity)
{
  transform_t transform;
  transform.translation = entity->position;
  transform.rotation = entity->rotation;
  return transform;
}

aabb3_t entity_hitbox(const struct entity *entity)
{
  const struct entity_info *info = query_entity_info(entity->id);

  aabb3_t hitbox;
  hitbox.center = fvec3_add(entity->position, info->hitbox_offset);
  hitbox.dimension = info->hitbox_dimension;
  return hitbox;
}

fvec3_t entity_local_to_global(struct entity *entity, fvec3_t vec)
{
  fvec4_t result;
  result = fvec4(vec.x, vec.y, vec.z, 1.0f);
  result = fmat4_mul_vec(fmat4_rotate(entity->rotation), result);
  return fvec3(result.x, result.y, result.z);
}

void entity_apply_impulse(struct entity *entity, fvec3_t impulse)
{
  entity->velocity = fvec3_add(entity->velocity, impulse);
}

void entity_move(struct entity *entity, fvec2_t direction, float speed, float dt)
{
  fvec3_t impulse;
  impulse = entity_local_to_global(entity, fvec3(direction.x, direction.y, 0.0f));
  impulse = fvec3(impulse.x, impulse.y, 0.0f);
  impulse = fvec3_normalize(impulse);
  impulse = fvec3_mul_scalar(impulse, speed);
  impulse = fvec3_mul_scalar(impulse, dt);
  entity_apply_impulse(entity, impulse);
}

void entity_jump(struct entity *entity, float strength)
{
  if(entity->grounded)
    entity->velocity.z = strength;
}

bool entity_ray_cast(struct entity *entity, float distance, ivec3_t *position, ivec3_t *normal)
{
  fvec3_t ray_position = entity->position;
  fvec3_t ray_direction = entity_local_to_global(entity, fvec3(0.0f, 1.0f, 0.0f));

  struct ray_cast ray_cast;
  ray_cast_init(&ray_cast, ray_position);

  *position = ray_cast.iposition;
  *normal= ivec3_zero();

  while(ray_cast.distance < distance)
  {
    const block_id_t block_id = world_get_block_id(ray_cast.iposition);
    if(block_id != BLOCK_NONE && query_block_info(block_id)->render_type == BLOCK_RENDER_TYPE_OPAQUE)
      return true;

    ray_cast_step(&ray_cast, ray_direction);
    *normal = ivec3_sub(*position, ray_cast.iposition);
    *position = ray_cast.iposition;
  }

  return false;
}

