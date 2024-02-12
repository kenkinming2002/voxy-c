#include <voxy/types/entity.h>
#include <voxy/types/world.h>
#include <voxy/types/block.h>

#include <voxy/main_game/mod.h>

#include <voxy/mod_interface.h>

#include <voxy/math/ray_cast.h>

fvec3_t entity_view_position(const struct entity *entity)
{
  return fvec3_add(entity->position, entity->local_view_transform.translation);
}

fvec3_t entity_view_rotation(const struct entity *entity)
{
  return entity->local_view_transform.rotation;
}

fvec3_t entity_view_direction(const struct entity *entity)
{
  return transform_forward(entity->local_view_transform);
}

transform_t entity_view_transform(const struct entity *entity)
{
  transform_t result;
  result.translation = fvec3_add(entity->position, entity->local_view_transform.translation);
  result.rotation    = entity->local_view_transform.rotation;
  return result;
}

void entity_apply_impulse(struct entity *entity, fvec3_t impulse)
{
  entity->velocity = fvec3_add(entity->velocity, impulse);
}

bool entity_ray_cast(struct entity *entity, struct world *world, float distance, ivec3_t *position, ivec3_t *normal)
{
  fvec3_t ray_position  = entity->position;
  fvec3_t ray_direction = transform_forward(entity->local_view_transform);

  struct ray_cast ray_cast;
  ray_cast_init(&ray_cast, ray_position);

  *position = ray_cast.iposition;
  *normal   = ivec3_zero();

  while(ray_cast.distance < distance)
  {
    struct block *block = world_get_block(world, ray_cast.iposition);
    if(block && mod_block_info_get(block->id)->type == BLOCK_TYPE_OPAQUE)
      return true;

    ray_cast_step(&ray_cast, ray_direction);
    *normal   = ivec3_sub(*position, ray_cast.iposition);
    *position = ray_cast.iposition;
  }
  return false;
}
