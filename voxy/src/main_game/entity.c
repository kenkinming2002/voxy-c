#include <voxy/main_game/entity.h>

#include <voxy/main_game/block.h>
#include <voxy/main_game/mod.h>
#include <voxy/main_game/mod_interface.h>
#include <voxy/main_game/world.h>

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

void entity_move(struct entity *entity, fvec2_t direction, float speed, float dt)
{
  fvec3_t impulse = fvec3_add(
    fvec3_mul_scalar(transform_right  (entity->local_view_transform), direction.x),
    fvec3_mul_scalar(transform_forward(entity->local_view_transform), direction.y)
  );

  impulse.z = 0.0f;
  impulse   = fvec3_normalize(impulse);
  impulse   = fvec3_mul_scalar(impulse, speed);
  impulse   = fvec3_mul_scalar(impulse, dt);

  entity_apply_impulse(entity, impulse);
}

void entity_jump(struct entity *entity, float strength)
{
  if(entity->grounded)
  {
    entity->grounded = false;
    entity_apply_impulse(entity, fvec3(0.0f, 0.0f, strength));
  }
}

bool entity_ray_cast(struct entity *entity, float distance, ivec3_t *position, ivec3_t *normal)
{
  fvec3_t ray_position  = entity->position;
  fvec3_t ray_direction = transform_forward(entity->local_view_transform);

  struct ray_cast ray_cast;
  ray_cast_init(&ray_cast, ray_position);

  *position = ray_cast.iposition;
  *normal   = ivec3_zero();

  while(ray_cast.distance < distance)
  {
    struct block *block = world_block_get(ray_cast.iposition);
    if(block && mod_block_info_get(block->id)->type == BLOCK_TYPE_OPAQUE)
      return true;

    ray_cast_step(&ray_cast, ray_direction);
    *normal   = ivec3_sub(*position, ray_cast.iposition);
    *position = ray_cast.iposition;
  }
  return false;
}
