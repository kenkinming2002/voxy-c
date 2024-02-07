#include "world_update_physics.h"

#include "world.h"
#include "resource_pack.h"
#include "entity.h"

#include <stdio.h>
#include <stdbool.h>

struct box
{
  fvec3_t position;
  fvec3_t dimension;
};

static inline fvec3_t box_point1(const struct box *box) { return fvec3_sub(box->position, fvec3_mul_scalar(box->dimension, 0.5f)); }
static inline fvec3_t box_point2(const struct box *box) { return fvec3_add(box->position, fvec3_mul_scalar(box->dimension, 0.5f)); }

static inline struct box box_expand(const struct box *box, fvec3_t offset)
{
  fvec3_t point1 = box_point1(box);
  fvec3_t point2 = box_point2(box);

  for(int i=0; i<3; ++i)
    if(signbit(offset.values[i]))
      point1.values[i] += offset.values[i];
    else
      point2.values[i] += offset.values[i];

  return (struct box){
    .position  = fvec3_mul_scalar(fvec3_add(point1, point2), 0.5f),
    .dimension = fvec3_sub(point2, point1),
  };
}

static void entity_physics_apply_law(struct entity *entity, float dt)
{
  entity->velocity = fvec3_add(entity->velocity, fvec3(0.0f, 0.0f, -PHYSICS_GRAVITY * dt));   // Gravity
  entity->velocity = fvec3_mul_scalar(entity->velocity, expf(-dt * (entity->grounded ? PHYSICS_DRAG_GROUND : PHYSICS_DRAG_AIR))); // Drag
}

static inline bool box_swept(const struct box *box1, const struct box *box2, fvec3_t offset, float *t, fvec3_t *normal)
{
  // We are essentially solving:
  //
  // ts[i] * offset[i] = box2_point1[i] - box1_point2[i]
  // ts[i] * offset[i] = box1_point2[i]   box2_point1[i]
  //
  // However, there is one slight problem.
  // It is possible we would have:
  //
  // ts[i] * 0 = 0
  //
  // A naive division will yield nan. We need to think about what is actually
  // happening in such a case. The two boxes are just
  // sliding along the boundary.
  //
  // We make the choice that this does not count as collision. This is because
  // that is the configuration we would have after resolving collision.
  fvec3_t ts_1 = fvec3_div(fvec3_sub(box_point1(box2), box_point2(box1)), offset);
  fvec3_t ts_2 = fvec3_div(fvec3_sub(box_point2(box2), box_point1(box1)), offset);
  for(int i=0; i<3; ++i)
    if(isnanf(ts_1.values[i]) || isnanf(ts_2.values[i]))
      return false;

  fvec3_t ts_near = fvec3_min(ts_1, ts_2);
  fvec3_t ts_far  = fvec3_max(ts_1, ts_2);

  float t_near = -INFINITY;
  float t_far  =  INFINITY;
  for(int i=0; i<3; ++i)
  {
    if(t_near < ts_near.values[i]) t_near = ts_near.values[i];
    if(t_far  > ts_far .values[i]) t_far  = ts_far .values[i];
  }
  if(t_near < 0.0f) t_near = 0.0f;
  if(t_far  > 1.0f) t_far  = 1.0f;

  if(t_near >= t_far)
    return false;

  for(int i=0; i<3; ++i)
    if(t_near == ts_near.values[i])
    {
      *t = t_near;
      *normal = fvec3_zero();
      normal->values[i] = signbit(offset.values[i]) ? 1.0f : -1.0f;
      return true;
    }

  // That means we are already colliding even without moving box1. That is - we
  // some how managed to get ourselves stuck. Do not attempt to resolve
  // collision.
  return false;
}

static void entity_physics_update(struct entity *entity, struct world *world, struct resource_pack *resource_pack, float dt)
{
  for(;;)
  {
    fvec3_t offset = fvec3_mul_scalar(entity->velocity, dt);

    struct box entity_box = { .position = entity->position, .dimension = entity->dimension };
    struct box entity_box_expanded = box_expand(&entity_box, offset);

    float   min_t = INFINITY;
    fvec3_t min_normal;

    ivec3_t point1 = fvec3_as_ivec3_round(box_point1(&entity_box_expanded));
    ivec3_t point2 = fvec3_as_ivec3_round(box_point2(&entity_box_expanded));
    for(int z=point1.z; z<=point2.z; ++z)
      for(int y=point1.y; y<=point2.y; ++y)
        for(int x=point1.x; x<=point2.x; ++x)
        {
          ivec3_t position = ivec3(x, y, z);

          struct tile *tile = world_get_tile(world, position);
          if(tile && resource_pack->block_infos[tile->id].type == BLOCK_TYPE_OPAQUE)
          {
            float   t;
            fvec3_t normal;
            if(box_swept(&entity_box, &(struct box){ .position = ivec3_as_fvec3(position), .dimension = fvec3(1.0f, 1.0f, 1.0f), }, offset, &t, &normal) && min_t > t)
            {
              min_t      = t;
              min_normal = normal;
            }
          }
        }

    if(isinff(min_t))
      return;

    if(min_normal.z == 1.0f)
      entity->grounded = true;

    entity->velocity = fvec3_sub(entity->velocity, fvec3_mul_scalar(min_normal, fvec3_dot(min_normal, entity->velocity) * (1-min_t)));
  }
}

static void entity_physics_integrate(struct entity *entity, float dt)
{
  entity->position = fvec3_add(entity->position, fvec3_mul_scalar(entity->velocity, dt));
}

void world_update_physics(struct world *world, struct resource_pack *resource_pack, float dt)
{
  entity_physics_apply_law(&world->player.base, dt);
  entity_physics_update(&world->player.base, world, resource_pack, dt);
  entity_physics_integrate(&world->player.base, dt);
}
