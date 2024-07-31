#include <voxy/scene/main_game/update/physics.h>

#include <voxy/scene/main_game/types/block.h>
#include <voxy/scene/main_game/types/entity.h>

#include <voxy/scene/main_game/states/chunks.h>

#include <voxy/scene/main_game/config.h>
#include <voxy/scene/main_game/mod.h>

#include <voxy/math/box.h>
#include <voxy/math/direction.h>

#include <voxy/core/log.h>

#include <stdbool.h>

static void entity_physics_apply_law(struct entity *entity, float dt)
{
  entity->velocity = fvec3_add(entity->velocity, fvec3(0.0f, 0.0f, -PHYSICS_GRAVITY * dt));   // Gravity
  entity->velocity = fvec3_mul_scalar(entity->velocity, expf(-dt * (entity->grounded ? PHYSICS_DRAG_GROUND : PHYSICS_DRAG_AIR))); // Drag
}

static bool intersect_intervals(float a1, float a2, float b1, float b2)
{
  return a2 > b1 && b2 > a1;
}

/// Return if there is any *t in the range [0.0, 1.0] such that if box where to
/// move by *t * offset, box would intersect with rect.
static bool swept_box_rect(box_t box, rect_t rect, fvec3_t offset, float *t)
{
  // The only possibility in such a case is if we hit the rect on its edge or
  // are already intersecting with the rect without moving, in in both of the
  // cases there is no resonable resolution and we might as well ignore it.
  if(offset.values[rect.axis] == 0.0f)
    return false;

  // Compute the time required for the box to maybe hit the rect.
  if(rect.center.values[rect.axis] >= box_max_corner(box).values[rect.axis] && offset.values[rect.axis] > 0.0f) {
    *t = (rect.center.values[rect.axis] - box_max_corner(box).values[rect.axis]) / offset.values[rect.axis];
  } else if(rect.center.values[rect.axis] <= box_min_corner(box).values[rect.axis] && offset.values[rect.axis] < 0.0f) {
    *t = (rect.center.values[rect.axis] - box_min_corner(box).values[rect.axis]) / offset.values[rect.axis];
  } else {
    // We are in a degnerate case here. If determine that we are colliding after
    // checking the other axis, we are stuck in a block. In that case, we want
    // to treat it as if there is no collision anyway so we may as well skip the
    // check early.
    return false;
  }

  if(*t > 1.0f)
    return false;

  // Adjust the box center according to the computed time and check if the box
  // really do hit the rect.
  box.center = fvec3_add(box.center, fvec3_mul_scalar(offset, *t));
  for(unsigned axis=0; axis<3; ++axis)
    if(axis != rect.axis)
    {
      const float a1 = box.center.values[axis] - 0.5f * box.dimension.values[axis];
      const float a2 = box.center.values[axis] + 0.5f * box.dimension.values[axis];

      const float b1 = rect.center.values[axis] - 0.5f * rect.dimension.values[axis];
      const float b2 = rect.center.values[axis] + 0.5f * rect.dimension.values[axis];

      if(!intersect_intervals(a1, a2, b1, b2))
        return false;
    }

  return true;
}

static void entity_physics_update(struct entity *entity, float dt)
{
  entity->grounded = false;
  for(;;)
  {
    bool hit = false;
    float min_t;
    direction_t min_direction;

    const fvec3_t offset = fvec3_mul_scalar(entity->velocity, dt);

    const struct entity_info *entity_info = query_entity_info(entity->id);
    const box_t entity_hitbox = box(fvec3_add(entity->position, entity_info->hitbox_offset), entity_info->hitbox_dimension);
    const box_t entity_hitbox_expanded = box_expand(entity_hitbox, offset);

    const ivec3_t block_position_min = fvec3_as_ivec3_round(box_min_corner(entity_hitbox_expanded));
    const ivec3_t block_position_max = fvec3_as_ivec3_round(box_max_corner(entity_hitbox_expanded));
    for(int z=block_position_min.z; z<=block_position_max.z; ++z)
      for(int y=block_position_min.y; y<=block_position_max.y; ++y)
        for(int x=block_position_min.x; x<=block_position_max.x; ++x)
        {
          const ivec3_t block_position = ivec3(x, y, z);
          const block_id_t block_id = world_get_block_id(block_position);
          if(block_id != BLOCK_NONE && query_block_info(block_id)->type == BLOCK_TYPE_OPAQUE)
            for(direction_t direction = 0; direction < DIRECTION_COUNT; ++direction)
            {
              const ivec3_t neighbour_block_position = ivec3_add(block_position, direction_as_ivec(direction));
              const block_id_t neighbour_block_id = world_get_block_id(neighbour_block_position);
              if(neighbour_block_id == BLOCK_NONE || query_block_info(neighbour_block_id)->type != BLOCK_TYPE_OPAQUE)
              {
                const box_t block_hitbox = box(ivec3_as_fvec3(block_position), fvec3(1.0f, 1.0f, 1.0f));
                const rect_t block_face_hitbox = box_face(block_hitbox, direction);

                float t;
                if(swept_box_rect(entity_hitbox, block_face_hitbox, offset, &t))
                  if(!hit || min_t > t)
                  {
                    hit = true;
                    min_t = t;
                    min_direction = direction;
                  }
              }
            }
        }

    // Update entity velocity accordingly if we hit something. We have to make
    // sure that we are actually updating something and not get stuck in an
    // infinite loop.
    if(hit && entity->velocity.values[direction_axis(min_direction)] != 0.0f && min_t != 1.0f)
    {
      entity->velocity.values[direction_axis(min_direction)] *= min_t;
      if(min_direction == DIRECTION_TOP)
        entity->grounded = true;
    }
    else
      break;
  }
}

static void entity_physics_integrate(struct entity *entity, float dt)
{
  entity->position = fvec3_add(entity->position, fvec3_mul_scalar(entity->velocity, dt));
}

static void entity_update_physics(struct entity *entity, float dt)
{
  entity_physics_apply_law(entity, dt);
  entity_physics_update(entity, dt);
  entity_physics_integrate(entity, dt);
}

void update_physics(float dt)
{
  world_for_each_chunk(chunk)
    if(chunk->data)
    {
      for(size_t i=0; i<chunk->data->entities.item_count; ++i)
        entity_update_physics(&chunk->data->entities.items[i], dt);

      if(chunk->data->entities.item_count != 0)
        chunk->data->dirty = true;
    }
}
