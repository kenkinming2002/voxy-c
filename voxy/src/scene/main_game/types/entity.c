#include <voxy/scene/main_game/types/entity.h>

#include <voxy/scene/main_game/types/block.h>
#include <voxy/scene/main_game/types/registry.h>

#include <voxy/scene/main_game/states/chunks.h>

#include <voxy/scene/main_game/mod.h>

#include <voxy/math/transform.h>

#include <voxy/math/ray_cast.h>

transform_t entity_transform(struct entity *entity)
{
  transform_t transform;
  transform.translation = entity->position;
  transform.rotation = entity->rotation;
  return transform;
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
  {
    entity->grounded = false;
    entity_apply_impulse(entity, fvec3(0.0f, 0.0f, strength));
  }
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
    if(block_id != BLOCK_NONE && query_block_info(block_id)->type == BLOCK_TYPE_OPAQUE)
      return true;

    ray_cast_step(&ray_cast, ray_direction);
    *normal = ivec3_sub(*position, ray_cast.iposition);
    *position = ray_cast.iposition;
  }

  return false;
}

struct hitbox
{
  fvec3_t position;
  fvec3_t dimension;
};

static struct hitbox entity_get_hitbox(struct entity *entity)
{
  const struct entity_info *entity_info = query_entity_info(entity->id);

  struct hitbox hitbox;
  hitbox.position = fvec3_add(entity->position, entity_info->hitbox_offset);
  hitbox.dimension = entity_info->hitbox_dimension;
  return hitbox;
}

bool entity_intersect(struct entity *entity1, struct entity *entity2)
{
  const struct hitbox hitbox1 = entity_get_hitbox(entity1);
  const struct hitbox hitbox2 = entity_get_hitbox(entity2);
  for(int i=0; i<3; ++i)
  {
    const float begin1 = hitbox1.position.values[i] - hitbox1.dimension.values[i] * 0.5f;
    const float end1   = hitbox1.position.values[i] + hitbox1.dimension.values[i] * 0.5f;

    const float begin2 = hitbox2.position.values[i] - hitbox2.dimension.values[i] * 0.5f;
    const float end2   = hitbox2.position.values[i] + hitbox2.dimension.values[i] * 0.5f;

    if(end1 < begin2 || end2 < begin1)
      return false;
  }
  return true;
}
